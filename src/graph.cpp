#include "graph.hpp"

namespace graph {
auto make_pos(const osmium::NodeRef& node) -> Position {
    return { node.lat(), node.lon() };
}

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

auto make_building(const osmium::Way& way) -> Building {
    const auto type = way.tags().get_value_by_key("building");
    const auto position = barycenter(way.nodes());
    const std::vector<std::string> houses =
        { "apartments", "bungalow", "cabin", "detached", "dormitory", "farm", "ger", "hotel",
          "house", "houseboat", "residential", " semidetached_house", "static_caravan", "terrace" };
    for (const auto& house: houses) {
        if (type == house) { return Building { position, 0, Building::Type::House }; }
    }
    return Building { position, 0, Building::Type::Facility };
}

auto import_map(osmium::io::File& file) -> Map {
    using NodesMarker = std::unordered_map<Node, bool>;
    using NodesLocation = std::unordered_map<Node, Position>;

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
        NodesLocation locations {};

        void way(const osmium::Way& way) noexcept {
            if (!way.tags().has_key("highway")) { return; }

            auto first = way.nodes().cbegin();
            auto last = way.nodes().crbegin();
            auto pred = first;

            for (const auto& node: way.nodes()) {
                locations.insert({ node.ref(), make_pos(node) });
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

    // TODO: Should handle locations properly, I used hash table to store them for now.
    // Probably should re-implement Node and define its hash function.
    // Boost hash is already imported, BTW.

    struct GraphHandler: public osmium::handler::Handler {
        Graph routes;
        NodesLocation locations;
        ClosestNode closest {};

        void way(const osmium::Way& way) noexcept {
            if (!way.tags().has_key("building")) { return; }

            auto building = make_building(way);
            // Get reference to the closest node
            auto node = std::min_element(routes.nodes().cbegin(), routes.nodes().cend(),
                                         [&](const auto& lhs, const auto& rhs) {
                                             return
                                                 haversine(locations[lhs.first], building.position)
                                                 <
                                                 haversine(locations[rhs.first], building.position);
                                         })->first;
            closest.emplace_back(building, node);
        }
    };

    using Index = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
    using LocationHandler = osmium::handler::NodeLocationsForWays<Index>;

    Index index;
    osmium::io::Reader cr { file }, fr { file }, gr { file };

    CountHandler ch;
    osmium::apply(cr, ch);

    FillHandler fh {{}, std::move(ch.marked) };
    LocationHandler lhf { index };
    osmium::apply(fr, lhf, fh);

    GraphHandler gh {{}, std::move(fh.routes), std::move(fh.locations) };
    LocationHandler lhg { index };
    osmium::apply(gr, lhg, gh);

    cr.close();
    fr.close();
    gr.close();

    return Map { gh.closest, gh.routes };
}
} // namespace graph
