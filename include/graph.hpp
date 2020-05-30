#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <utility>
#include <vector>
#include <memory>
#include <queue>

#include <boost/functional/hash.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace graph {
using Distance = double;
using Angle = long double;
using Position = std::pair<Angle, Angle>;

using Node = std::uint64_t;
using Edge = std::pair<Node, Node>;

struct Building {
    Building() = default;

    Building(Position p, Distance w, unsigned char t)
        : p(std::move(p))
        , w(w) {
        if (t == 0) { type = Type::House; }
        else { type = Type::Facility; }
    }

    bool operator==(const Building& other) const {
        return type == other.type && p == other.p && w == other.w;
    }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& archive, const unsigned int& version) {
        (void) version;
        archive & p;
        archive & w;
        archive & type;
    }

    [[nodiscard]] bool is_house() const { return type == Type::House; }
    [[nodiscard]] bool is_facility() const { return type == Type::Facility; }

    [[nodiscard]] Position pos() const { return p; }
    [[nodiscard]] auto x() const { return p.first; }
    [[nodiscard]] auto y() const { return p.second; }
    [[nodiscard]] auto weight() const { return w; }

private:
    enum class Type {
        House,
        Facility
    };

    Position p = { 0, 0 };
    Distance w = 0;
    Type type = Type::House;
};

using Nodes = std::vector<Node>;
using Buildings = std::vector<Building>;
} // namespace graph

namespace std {
template<>
struct hash<graph::Building> {
    size_t operator()(const graph::Building& b) const {
        using boost::hash_value;
        using boost::hash_combine;

        size_t seed = 0;
        hash_combine(seed, hash_value(b.x()));
        hash_combine(seed, hash_value(b.y()));
        hash_combine(seed, hash_value(b.weight()));
        return seed;
    }
};
} // namespace std

namespace graph {
/**
 * Type for mapping Building to the closest route Node.
 */
using ClosestNode = std::unordered_map<Building, Node>;

/**
 * Type for representing shortest paths from one Node to others.
 */
using ShortestPaths = std::unordered_map<Node, Distance>;

/**
 * Undirected weighted routing graph implemented as an adjacency list.
 */
struct Graph {
private:
    using OutgoingEdges = std::unordered_map<Node, Distance>;
    using AdjacencyList = std::unordered_map<Node, OutgoingEdges>;

public:
    bool add_edge(Edge&& e, Distance d = 0) noexcept;

    bool serialize(const std::string& filename = "graph.bin") const;
    bool deserialize(const std::string& filename = "graph.bin");

    const auto& nodes() const { return data; }

    auto dijkstra(Node s) const -> ShortestPaths;

private:
    AdjacencyList data {};
};

/**
 * Data structure envision:
 *
 *             Building    Node    (references)    Node maps Distance
 * ClosestNode a        -> x    -> UWGraph      -> [(x1 -> w), (x2 -> w), (x3 -> w)]
 * ClosestNode b        -> y    -> UWGraph      -> [(y1 -> w)]
 * ClosestNode c        -> z    -> UWGraph      -> [(z1 -> w), (z2 -> w)]
 *
 * where {a, b, c} -- Buildings, {x, y, z} -- Nodes, w -- Distances.
 */
struct Map {
    Map() = default;

    Map(ClosestNode c, Graph g)
        : building_to_node(std::move(c))
        , graph(std::move(g)) {};

    /**
     * Select buildings by applying functor to each.
     *
     * @param functor [](const Building&) { return true; }
     * @return Vector of corresponding nodes.
     */
    template<typename F>
    auto select_buildings(F&& functor) const -> std::vector<Node>;

    /**
     * Select N buildings and apply functor to each.
     *
     * @param num Number of building to select.
     * @param functor [](const Buildings&) { return true; }
     * @return Vector of corresponding nodes.
     */
    template<typename F>
    auto select_random_buildings(size_t num, F&& functor) const -> ClosestNode;
    auto select_random_facilities(size_t num) const -> ClosestNode;
    auto select_random_houses(size_t num) const -> ClosestNode;

    /**
     * Get shortest paths from Node to all other Nodes specified.
     *
     * @return Mapping from Nodes to the corresponding paths from the given Node.
     */
    auto shortest_paths(const Building& from, const ClosestNode& to) const -> ShortestPaths;

    bool serialize(const std::string& filename = "map.bin");
    bool deserialize(const std::string& filename = "map.bin");

    const auto& buildings() const { return building_to_node; }
    const auto& nodes() const { return graph.nodes(); }

private:
    ClosestNode building_to_node {};
    Graph graph {};
};

/**
 * Constructs routing graph based on provided OSM geodata.
 *
 * @param file File with geographic data.
 * @return Constructed routing graph and the list of buildings.
 */
auto import_map(const std::string& filename) -> Map;

/**
 * Determines the great-circle distance between two points given their longitudes and latitudes.
 *
 * @param x, y OSM nodes with corresponding coordinates.
 * @return Distance between nodes.
 */
auto haversine(const Position& x, const Position& y) -> Distance;
} // namespace graph

#endif // GRAPH_HPP
