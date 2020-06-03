#include "map.hpp"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <numeric>
#include <random>
#include <filesystem>

#include <boost/iostreams/stream.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/pbf_input.hpp>

namespace graphs {
template<typename F>
auto Map::select_buildings(F&& functor) const -> Buildings {
    Buildings result {};
    for (const auto& building: m_buildings) {
        if (functor(building)) { result.push_back(building); }
    }
    return result;
};

template<typename F>
auto Map::select_random_buildings(size_t num, F&& functor) const -> Buildings {
    Buildings buildings {}, result {};
    for (const auto& building: m_buildings) {
        if (functor(building)) {
            buildings.push_back(building);
        }
    }
    if (buildings.empty()) { return buildings; }
    std::sample(buildings.cbegin(), buildings.cend(), std::inserter(result, result.end()), num,
                std::mt19937 { std::random_device {}() });
    return result;
}

auto Map::select_random_facilities(size_t num) const -> Buildings {
    return select_random_buildings(num, [](const auto& b) {
        return b.is_facility();
    });
};

auto Map::select_random_houses(size_t num) const -> Buildings {
    return select_random_buildings(num, [](const auto& b) {
        return b.is_house();
    });
};

bool Map::serialize(const std::string& filename) const {
    return ::graphs::serialize(filename, m_buildings) && m_graph.serialize();
};

bool Map::deserialize(const std::string& filename) {
    if (!std::filesystem::exists(filename)) { return false; }
    return ::graphs::deserialize(filename, m_buildings) && m_graph.deserialize();
};

auto Map::shortest_paths(Building from, const Buildings& to) const -> Paths {
    auto[distances, trail] = m_graph.dijkstra(from.closest());
    Paths result {};

    for (const auto& building: to) {
        auto node_to = building.closest();
        auto distance = distances.at(node_to);
        result.emplace_back(from, building, distance);
    }

    return result;
}

auto Map::shortest_paths_with_trace(Building from, const Buildings& to) const -> TracedPaths {
    auto[distances, trail] = m_graph.dijkstra(from.closest());
    TracedPaths result {};

    for (const auto& building: to) {
        auto node_to = building.closest(), node_from = from.closest();
        auto distance = distances.at(node_to);

        // Reconstruct path.
        std::vector<Node> path;
        for (auto v = node_to; v != from.closest(); v = trail[v]) {
            path.push_back(v);
        }
        path.push_back(node_from);
        reverse(path.begin(), path.end());

        // Build path in place.
        result.emplace_back(from, building, path, distance);
    }

    return result;
}

auto Map::weights_sum() const -> long double {
    return std::accumulate(nodes().cbegin(), nodes().cend(), static_cast<long double>(0),
                           [](auto lhs, const auto& node) {
                               return lhs +
                                      std::accumulate(node.second.cbegin(), node.second.cend(),
                                                      static_cast<long double>(0),
                                                      [](auto lhs, const auto& edge) {
                                                          return lhs + edge.second;
                                                      });
                           });
}

Map paths_to_map(const Map& map, const Map::TracedPaths& paths) {
    std::unordered_set<Building> set;
    Buildings buildings;
    Graph routes;

    for (const auto& path: paths) {
        auto[from, to] = path.ends();
        set.insert(from);
        set.insert(to);

        auto pred = *path.path().begin();
        for (const auto curr: path.path()) {
            if (curr == pred) { continue; }
            auto weight = map.nodes().find(pred)->second.find(curr)->second;
            routes.add_edge_one_way({ pred, curr }, weight);
            pred = curr;
        }
    }

    buildings.reserve(set.size());
    for (auto b: set) {
        buildings.push_back(set.extract(b).value());
    }

    return Map { buildings, routes };
}

std::optional<Map> import_map(const std::string& filename, bool recache) {
    using NodesMarker = std::unordered_map<std::uint64_t, bool>;
    using NodesLocation = std::unordered_map<Node, Location>;

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
            bool one_way = way.tags().has_tag("oneway", "yes");

            auto first = way.nodes().cbegin();
            auto last = way.nodes().crbegin();
            auto pred = first;

            for (const auto& node: way.nodes()) {
                Node current { node.positive_ref(), node.lon(), node.lat() };
                locations.insert({ current, make_pos(node) });
                if (node.ref() != first->ref() && node.ref() != last->ref()) {
                    if (marked.at(node.ref())) {
                        // Split up the way
                        const auto distance = haversine(make_pos(*pred), make_pos(node));
                        Node previous { pred->positive_ref(), pred->lon(), pred->lat() };
                        if (one_way) {
                            routes.add_edge_one_way({ previous, current }, distance);
                        } else {
                            routes.add_edge_two_way({ previous, current }, distance);
                        }
                        pred = &node;
                    }
                }
            }

            Node previous { pred->positive_ref(), pred->lon(), pred->lat() };
            Node end { last->positive_ref(), last->lon(), last->lat() };
            const auto distance = haversine(make_pos(*pred), make_pos(*last));
            if (one_way) {
                routes.add_edge_one_way({ previous, end }, distance);
            } else {
                routes.add_edge_two_way({ previous, end }, distance);
            }
        }
    };

    struct MapHandler: public osmium::handler::Handler {
        Graph routes;
        NodesLocation locations;
        Buildings buildings {};

        void way(const osmium::Way& way) noexcept {
            if (!way.tags().has_key("building")) { return; }

            const auto location = barycenter(way.nodes());
            // Get reference to the closest node
            auto node = std::min_element(routes.nodes().cbegin(), routes.nodes().cend(),
                                         [&](const auto& lhs, const auto& rhs) {
                                             return
                                                 haversine(locations[lhs.first], location) <
                                                 haversine(locations[rhs.first], location);
                                         })->first;
            auto building = make_building(way, location, node);
            buildings.push_back(building);
        }
    };

    using Index = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
    using LocationHandler = osmium::handler::NodeLocationsForWays<Index>;

    /*
     * If using cached map, return.
     */
    if (!recache) {
        Map map {};
        if (map.deserialize()) { return map; }
        else { return std::nullopt; }
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
    Map map { gh.buildings, gh.routes };
    map.serialize();

    return map;
}
} // namespace graphs
