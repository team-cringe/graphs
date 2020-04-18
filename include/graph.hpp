#ifndef GRAPHS_GRAPH_HPP
#define GRAPHS_GRAPH_HPP

#include <set>
#include <osmium/visitor.hpp>

using std::set;

class Node {
    uint64_t _id;
public:
    explicit Node(uint64_t id) : _id(id) {}

    bool operator<(const Node &other) const {
        return _id < other._id;
    }

    [[nodiscard]]
    uint64_t id() const {
        return _id;
    }
};

class Edge {
    uint64_t _from;
    uint64_t _to;
public:
    explicit Edge(Node &from, Node &to) : _from(from.id()), _to(to.id()) {}

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

std::ostream &operator<<(std::ostream &out, const Edge &edge);

class Graph {
    set<Node> _nodes;
    set<Edge> _edges;
public:
    explicit Graph() : _nodes(set<Node>()), _edges(set<Edge>()) {}

    bool add_node(uint64_t id);

    bool add_edge(uint64_t from, uint64_t to);

    [[nodiscard]]
    const set<Node> &nodes() const {
        return _nodes;
    }


    [[nodiscard]]
    const set<Edge> &edges() const {
        return _edges;
    }

    static Graph from_map(osmium::io::File &);
};

#endif //GRAPHS_GRAPH_HPP
