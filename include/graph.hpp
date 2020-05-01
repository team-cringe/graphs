#ifndef GRAPHS_GRAPH_HPP
#define GRAPHS_GRAPH_HPP

#include <ostream>
#include <vector>
#include <osmium/visitor.hpp>

using AdjacencyList = std::vector<uint64_t>;

class Node {
    uint64_t _id_osm;
    mutable AdjacencyList _neighbors;

public:
    explicit Node(uint64_t id_osm)
        : _id_osm(id_osm)
        , _neighbors(AdjacencyList()) {}

    bool operator<(const Node& other) const {
        return _id_osm < other._id_osm;
    }

    [[nodiscard]]
    uint64_t id_osm() const {
        return _id_osm;
    }

    [[nodiscard]]
    const AdjacencyList& neighbors() const {
        return _neighbors;
    }

    friend class Graph;
};

class Graph {
    std::vector<Node> _nodes;
public:
    explicit Graph()
        : _nodes(std::vector<Node>()) {}

    explicit Graph(std::vector<Node>&& nodes): _nodes(nodes) {}

    bool add_node(uint64_t id_osm);

    bool add_edge(uint64_t from, uint64_t to);

    [[nodiscard]]
    const std::vector<Node>& nodes() const {
        return _nodes;
    }

    static Graph from_osm(osmium::io::File&);
};

#endif //GRAPHS_GRAPH_HPP
