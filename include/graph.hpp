#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "node.hpp"

namespace fs = std::filesystem;

namespace graphs {
/**
 * Type for representing shortest paths from one Node to others.
 */
using ShortestPaths = std::unordered_map<Node, Distance>;

/**
 * Undirected weighted routing graph implemented as an adjacency list.
 */
struct Graph {
private:
    using Edge = std::pair<Node, Node>;
    using OutgoingEdges = std::unordered_map<Node, Distance>;
    using AdjacencyList = std::unordered_map<Node, OutgoingEdges>;
    using Trail = std::unordered_map<Node, Node>;

public:
    bool add_edge_one_way(Edge&& e, Distance d = 0) noexcept;
    bool add_edge_two_way(Edge&& e, Distance d = 0) noexcept;

    bool serialize(const fs::path& filename) const;
    bool deserialize(const fs::path& filename);

    const auto& nodes() const { return m_data; }

    auto dijkstra(Node s) const -> std::pair<ShortestPaths, Trail>;

private:
    AdjacencyList m_data {};
};
} // namespace graph

#endif // GRAPH_HPP
