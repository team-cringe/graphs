#include <functional>
#include <numeric>

#include "graph.hpp"

namespace graph {
auto make_pos(const osmium::NodeRef& node) -> Position {
    return { node.lat(), node.lon() };
};

auto haversine(const Position& x, const Position& y) -> Distance {
    const auto[lat1, lon1] = x;
    const auto[lat2, lon2] = y;

    constexpr long double R = 6'371'000;

    // Convert to radians
    const auto phi1 = lat1 * M_PI / 180;
    const auto phi2 = lat2 * M_PI / 180;
    const auto d_phi = (lat2 - lat1) * M_PI / 180;
    const auto d_lambda = (lon2 - lon1) * M_PI / 180;

    // Square of half the chord length between the objects
    const auto a = std::pow(std::sin(d_phi / 2), 2) +
                   std::cos(phi1) * std::cos(phi2) * std::pow(std::sin(d_lambda / 2), 2);
    // Angular distance in radians
    const auto c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

auto barycenter(const osmium::WayNodeList& nodes) -> Position {
    const auto lat = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lat(); });
    const auto lon = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lon(); });
    const auto num = nodes.size();

    return { lat / num, lon / num };
}

auto import(osmium::io::File& file) -> Graph {
    using NodesMarker = std::unordered_map<Node, bool>;
    using Index = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
    using NearestNode = std::unordered_map<Position, Node, boost::hash<Position>>;
    using LocationHandler = osmium::handler::NodeLocationsForWays<Index>;

    struct CountHandler: public osmium::handler::Handler {
        NodesMarker marked {};

        void way(const osmium::Way& way) noexcept {
            // Throw away unrelated nodes
            if (!way.tags().has_key("highway")) { return; }

            for (const auto& node: way.nodes()) {
                // If the node lies on an intersection, mark it
                if (marked.find(node.ref()) != marked.end()) {
                    marked[node.ref()] |= true;
                } else {
                    marked.insert({ node.ref(), false });
                }
            }
        }
    };

    struct FillHandler: public osmium::handler::Handler {
        NodesMarker marked;
        Graph routes {};

        void way(const osmium::Way& way) noexcept {
            if (!way.tags().has_key("highway")) { return; }

            auto first = way.nodes().cbegin();
            auto last = way.nodes().crbegin();
            auto pred = first;

            for (const auto& node: way.nodes()) {
                if (node.ref() != first->ref() && node.ref() != last->ref()) {
                    if (marked.at(node.ref())) {
                        // Split up the way
                        const auto distance = haversine(make_pos(*pred), make_pos(node));
                        routes.add_edge({ pred->ref(), node.ref() }, distance);
                        pred = &node;
                    }
                }
            }

            const auto distance = haversine(make_pos(*pred), make_pos(*last));
            routes.add_edge({ pred->ref(), last->ref() }, distance);
        }
    };

    struct GraphHandler: public osmium::handler::Handler {
        Graph routes;
        NearestNode nearest {};

        void way(const osmium::Way& way) noexcept {
            if (!way.tags().has_key("building")) { return; }
            auto position = barycenter(way.nodes());
            auto closest = std::min_element(routes.nodes().cbegin(), routes.nodes().cend(),
                                            [&position](const auto& lhs, const auto& rhs) {
                                                return haversine(make_pos(lhs.first), position) <
                                                       haversine(make_pos(rhs.first), position);
                                            });
            nearest.insert({ position, closest->first });
        }
    };

    auto mode = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;

    Index index;
    osmium::io::Reader count_reader { file, mode };
    CountHandler count_handler;
    LocationHandler location_handler { index };
    osmium::apply(count_reader, count_handler);

    osmium::io::Reader fill_reader { file, mode };
    FillHandler fill_handler {{}, std::move(count_handler.marked) };
    osmium::apply(fill_reader, location_handler, fill_handler);

    osmium::io::Reader graph_reader { file, mode };
    GraphHandler graph_handler {{}, std::move(fill_handler.routes) };
    osmium::apply(graph_reader, location_handler, graph_handler);

    count_reader.close();
    fill_reader.close();
    graph_reader.close();

    return fill_handler.routes;
}
} // namespace graph
