#ifndef GRAPHS_GRAPH_HPP
#define GRAPHS_GRAPH_HPP

#include <ostream>
#include <set>
#include <osmium/visitor.hpp>

using std::set;

class Edge {
    uint64_t _from;
    uint64_t _to;
public:
    explicit Edge(uint64_t from, uint64_t to) : _from(from), _to(to) {}

    bool operator<(const Edge &other) const;

    [[nodiscard]]
    uint64_t from() const {
        return _from;
    }

    [[nodiscard]]
    uint64_t to() const {
        return _to;
    }
};

using AdjacencyList = set<Edge>;

class Node {
    uint64_t _id;
    mutable AdjacencyList _neighbors;

public:
    explicit Node(uint64_t id) : _id(id), _neighbors(AdjacencyList()) {}

    bool operator<(const Node &other) const {
        return _id < other._id;
    }

    [[nodiscard]]
    uint64_t id() const {
        return _id;
    }

    [[nodiscard]]
    const AdjacencyList &neighbors() const {
        return _neighbors;
    }

    friend class Graph;
};

std::ostream &operator<<(std::ostream &out, const Edge &edge);

class Graph {
    set<Node> _nodes;
    uint64_t _edges;
public:
    explicit Graph() : _nodes(set<Node>()), _edges(0) {}

    bool add_node(uint64_t id);

    bool add_edge(uint64_t from, uint64_t to);

    [[nodiscard]]
    const set<Node> &nodes() const {
        return _nodes;
    }

    [[nodiscard]]
    uint64_t edges() const {
        return _edges;
    }

    static Graph from_map(osmium::io::File &);
};

#endif //GRAPHS_GRAPH_HPP
