#include "graph.hpp"

#include <cmath>
#include <unordered_map>
#include <functional>
#include <numeric>
#include <random>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/pbf_input.hpp>

namespace graph {
template<typename T>
bool serialize(const std::string& filename, T&& data) {
    std::ofstream binary { filename, std::ios::out | std::ios::binary | std::ios::app };
    boost::archive::binary_oarchive archive { binary, boost::archive::no_header };
    archive << data;
    binary.close();

    return true;
}

template<typename T>
bool deserialize(const std::string& filename, T&& data) {
    if (!std::filesystem::exists(filename)) { return false; }

    std::ifstream binary { filename, std::ios::binary };
    boost::archive::binary_iarchive archive { binary, boost::archive::no_header };
    archive >> data;
    binary.close();

    return true;
}

/*
 * Graph structure declaration.
 */
bool Graph::add_edge(Edge&& e, Distance d) noexcept {
    auto[from, to] = e;
    if (from == to) { return false; }
    return data[from].insert({ to, d }).second && data[to].insert({ from, d }).second;
};

bool Graph::serialize(const std::string& filename) const {
    return ::graph::serialize(filename, data);
}

bool Graph::deserialize(const std::string& filename) {
    if (!std::filesystem::exists(filename)) { return false; }
    return ::graph::deserialize(filename, data);
}

/*
 * Map structure declaration.
 */
template<typename F>
auto Map::select_buildings(F&& functor) const -> std::vector<Node> {
    std::vector<Node> result {};
    for (const auto&[building, node]: closest) {
        if (functor(building)) { result.push_back(node); }
    }
    return result;
};

template<typename F>
auto Map::select_random_buildings(size_t num, F&& functor) const -> ClosestNode {
    ClosestNode buildings {}, result {};
    std::copy_if(closest.cbegin(), closest.cend(), std::inserter(buildings, buildings.end()),
                 std::forward<F>(functor));
    if (buildings.empty()) { return buildings; }
    std::sample(buildings.cbegin(), buildings.cend(), std::inserter(result, result.end()), num,
                std::mt19937 { std::random_device {}() });
    return result;
}

auto Map::select_random_facilities(size_t num) const -> ClosestNode {
    return select_random_buildings(num, [](const auto& p) {
        const auto[building, _] = p;
        return building.is_facility();
    });
};

auto Map::select_random_houses(size_t num) const -> ClosestNode {
    return select_random_buildings(num, [](const auto& p) {
        const auto[building, _] = p;
        return building.is_house();
    });
};

bool Map::serialize(const std::string& filename) {
    return ::graph::serialize(filename, closest) && graph.serialize("graph.bin");
};

bool Map::deserialize(const std::string& filename) {
    if (!std::filesystem::exists(filename)) { return false; }
    return ::graph::deserialize(filename, closest) &&
           graph.deserialize("graph.bin");
};

/**
 * Factory method for Position.
 */
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

/**
 * Determines the geographical center of a building consisting of ambient nodes.
 *
 * @param nodes List of nodes of an OSM way.
 * @return Geocenter described by a pair of latitude and longitude respectively.
 */
auto barycenter(const osmium::WayNodeList& nodes) -> Position {
    const auto lat = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lat(); });
    const auto lon = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lon(); });
    const auto num = nodes.size();

    return { lat / num, lon / num };
}

/**
 * Factory method for Building.
 * Calculates its position and resolves correct type based on OSM data.
 *
 * @details Use RTTI in order to define correct type in hierarchy.
 *
 * @param way OSM way that represents building (or you gonna catch runtime error, lol).
 */
auto make_building(const osmium::Way& way) -> Building {
    const auto type = way.tags().get_value_by_key("building");
    const auto position = barycenter(way.nodes());
    const std::vector<std::string> houses =
        { "apartments", "bungalow", "cabin", "detached", "dormitory", "farm", "ger", "hotel",
          "house", "houseboat", "residential", " semidetached_house", "static_caravan", "terrace" };
    for (const auto& house: houses) {
        if (type == house) { return Building { position, 0, 0 }; }
    }
    return Building { position, 0, 1 };
}

auto import_map(const std::string& filename) -> Map {
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

    struct GraphHandler: public osmium::handler::Handler {
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

    struct MapHandler: public osmium::handler::Handler {
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
                                                 haversine(locations[lhs.first], building.pos()) <
                                                 haversine(locations[rhs.first], building.pos());
                                         })->first;
            closest.insert({ building, node });
        }
    };

    using Index = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
    using LocationHandler = osmium::handler::NodeLocationsForWays<Index>;

    // If map is already cached, return
    {
        Graph routes {};
        Map map {};
        if (map.deserialize()) {
            return map;
        }
    }

    osmium::io::File file { filename };
    Index index;
    osmium::io::Reader cr { file }, fr { file }, gr { file };

    CountHandler ch;
    osmium::apply(cr, ch);

    GraphHandler fh {{}, std::move(ch.marked) };
    LocationHandler lhf { index };
    osmium::apply(fr, lhf, fh);

    MapHandler gh {{}, std::move(fh.routes), std::move(fh.locations) };
    LocationHandler lhg { index };
    osmium::apply(gr, lhg, gh);

    cr.close();
    fr.close();
    gr.close();

    // Create map and serialize
    Map map { gh.closest, gh.routes };
    map.serialize();

    return map;
}
} // namespace graph
