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

    Building(Position position, Distance weight, unsigned char type)
        : m_position(std::move(position))
        , m_weight(weight) {
        if (type == 0) { m_type = Type::House; }
        else { m_type = Type::Facility; }
    }

    bool operator==(const Building& other) const {
        return m_type == other.m_type && m_position == other.m_position && m_weight == other.m_weight;
    }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& archive, const unsigned int& version) {
        (void) version;
        archive & m_position;
        archive & m_weight;
        archive & m_type;
    }

    [[nodiscard]] bool is_house() const { return m_type == Type::House; }
    [[nodiscard]] bool is_facility() const { return m_type == Type::Facility; }

    [[nodiscard]] Position pos() const { return m_position; }
    [[nodiscard]] auto x() const { return m_position.first; }
    [[nodiscard]] auto y() const { return m_position.second; }
    [[nodiscard]] auto weight() const { return m_weight; }

private:
    enum class Type {
        House,
        Facility
    };

    Position m_position = { 0, 0 };
    Distance m_weight = 0;
    Type m_type = Type::House;
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
    using Trail = std::unordered_map<Node, Node>;

public:
    bool add_edge_one_way(Edge&& e, Distance d = 0) noexcept;
    bool add_edge_two_way(Edge&& e, Distance d = 0) noexcept;

    bool serialize(const std::string& filename = "graph.bin") const;
    bool deserialize(const std::string& filename = "graph.bin");

    const auto& nodes() const { return m_data; }

    auto dijkstra(Node s) const -> std::pair<ShortestPaths, std::unordered_map<Node, Node>>;

private:
    AdjacencyList m_data {};
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

    Map(ClosestNode closest, Graph graph)
        : m_closest(std::move(closest))
        , m_graph(std::move(graph)) {};

    struct Path {
        Path(Building from, Building to, Nodes path, Distance distance)
            : m_from(std::move(from))
            , m_to(std::move(to))
            , m_path(std::move(path))
            , m_distance(distance) {};

        [[nodiscard]] auto destinations() const -> std::pair<Building, Building> {
            return { m_from, m_to };
        }
        [[nodiscard]] const Nodes& path() const { return m_path; }
        [[nodiscard]] Distance distance() const { return m_distance; }

    private:
        Building m_from, m_to;
        Nodes m_path;
        Distance m_distance;
    };

    using Paths = std::vector<Path>;

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
    auto select_random_buildings(size_t num, F&& functor) const -> Buildings;
    auto select_random_facilities(size_t num) const -> Buildings;
    auto select_random_houses(size_t num) const -> Buildings;

    /**
     * Get shortest paths from Node to all other Nodes specified.
     *
     * @return Mapping from Nodes to the corresponding paths from the given Node.
     */
    auto shortest_paths(Building from, const Buildings& to) const -> std::vector<Path>;

    bool serialize(const std::string& filename = "map.bin") const;
    bool deserialize(const std::string& filename = "map.bin");

    const auto& buildings() const { return m_closest; }
    const auto& nodes() const { return m_graph.nodes(); }

private:
    ClosestNode m_closest {};
    Graph m_graph {};
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
