#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <cmath>
#include <unordered_map>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>
#include <random>
#include <iostream>

#include <boost/functional/hash.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

// TODO: This is totally disgusting. Sorry.

namespace graph {
using Distance = double;
using Angle = long double;
using Position = std::pair<Angle, Angle>;

using Node = std::uint64_t;
using Edge = std::pair<Node, Node>;

struct Building {
    enum class Type {
        House,
        Facility
    };

    bool is_house() const { return type == Type::House; }

    bool is_facility() const { return type == Type::Facility; }

    Position position = { 0, 0};
    Distance distance = 0;
    Type type;
};

/**
 * Type for mapping building to the closest route node.
 */
using ClosestNode = std::vector<std::pair<Building, Node>>;

/**
 * Undirected weighted routing graph implemented as an adjacency list.
 */
struct Graph {
private:
    using OutgoingEdges = std::unordered_map<Node, Distance>;
    using AdjacencyList = std::unordered_map<Node, OutgoingEdges>;

public:
    bool add_edge(Edge&& e, Distance d = 0) noexcept {
        auto[from, to] = e;
        if (from == to) { return false; }
        return data[from].insert({ to, d }).second && data[to].insert({ from, d }).second;
    }

    auto nodes() const {
        return data;
    }

private:
    AdjacencyList data {};
};

struct Map {
    Map(ClosestNode c, Graph g)
        : closest(std::move(c))
        , graph(std::move(g)) {};

    /**
     * Select buildings by applying functor to each.
     *
     * @param functor [](const Building&) { return true; }
     * @return Vector of corresponding nodes.
     */
    template<typename F>
    auto select_buildings(F functor) -> std::vector<Node> {
        std::vector<Node> result {};
        for (const auto&[building, node]: closest) {
            if (functor(building)) { result.push_back(node); }
        }
        return result;
    };

    /**
     * Select N buildings and apply functor to each.
     *
     * @param num Number of building to select.
     * @param functor [](const Buildings&) { return true; }
     * @return Vector of corresponding nodes.
     */
    template<typename F>
    auto select_random_buildings(std::size_t num, F&& functor) -> ClosestNode {
        ClosestNode buildings {}, result {};
        std::copy_if(closest.cbegin(), closest.cend(), std::back_inserter(buildings),
                     std::forward<F>(functor));
        std::sample(buildings.cbegin(), buildings.cend(), std::back_inserter(result), num,
                    std::mt19937 { std::random_device {}() });
        return result;
    }

    auto select_random_facilities(std::size_t num) -> ClosestNode {
        return select_random_buildings(num, [](const auto& p) {
            const auto[building, _] = p;
            return building.is_facility();
        });
    };

    auto select_random_houses(std::size_t num) -> ClosestNode {
        return select_random_buildings(num, [](const auto& p) {
            const auto[building, _] = p;
            return building.is_house();
        });
    };

    auto buildings() {
        return closest;
    }

    auto routes() {
        return graph;
    }

private:
    ClosestNode closest;
    Graph graph;
};

/**
 * Factory method for Position.
 */
auto make_pos(const osmium::NodeRef& node) -> Position;

/**
 * Factory method for Building.
 * Calculates its position and resolves correct type based on OSM data.
 *
 * @details Use RTTI in order to define correct type in hierarchy.
 *
 * @param way OSM way that represents building (or you gonna catch runtime error, lol).
 */
auto make_building(const osmium::Way& way) -> Building;

/**
 * Constructs routing graph based on provided OSM geodata.
 * Resulting data is envisioned bellow:
 *
 *             Building    Node    (references)    Node maps Distance
 * ClosestNode a        -> x    -> UWGraph      -> [(x1 -> w), (x2 -> w), (x3 -> w)]
 * ClosestNode b        -> y    -> UWGraph      -> [(y1 -> w)]
 * ClosestNode c        -> z    -> UWGraph      -> [(z1 -> w), (z2 -> w)]
 *
 * where {a, b, c} -- Buildings, {x, y, z} -- Nodes, w -- Distances.
 *
 * @param file File with geographic data.
 * @return Constructed routing graph and the list of buildings.
 */
auto import_map(osmium::io::File& file) -> Map;

/**
 * Determines the great-circle distance between two points given their longitudes and latitudes.
 *
 * @param x, y OSM nodes with corresponding coordinates.
 * @return Distance between nodes.
 */
auto haversine(const Position& x, const Position& y) -> Distance;

/**
 * Determines the geographical center of a building consisting of ambient nodes.
 *
 * @param nodes List of nodes of an OSM way.
 * @return Geocenter described by a pair of latitude and longitude respectively.
 */
auto barycenter(const osmium::WayNodeList& nodes) -> Position;
} // namespace graph

#endif // GRAPH_HPP
