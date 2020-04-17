#ifndef GRAPHS_GRAPH_HPP
#define GRAPHS_GRAPH_HPP

#include <set>
#include <osmium/visitor.hpp>

using std::set;

class Node {
    uint64_t _id;
public:
    explicit Node(uint64_t id): _id(id) {}

    bool operator<(const Node& other) const {
        return _id < other._id;
    }
};

class Edge {
    Node& _from;
    Node& _to;
public:
    explicit Edge(Node& from, Node& to): _from(from), _to(to) {}

    bool operator<(const Edge& other) const {
        return _from < other._from
        ? true
        : other._from < _from
        ? false
        : _to < other._to;
    }
};

class Graph {
    set<Node> _nodes;
    set<Edge> _edges;
public:
    explicit Graph(): _nodes(set<Node>()), _edges(set<Edge>()) {}
    const Node& add_node(uint64_t id);
    void add_edge(Node& from, Node& to);

    [[nodiscard]]
    const set<Node>& nodes() const;

    [[nodiscard]]
    const set<Edge>& edges() const;

    static Graph from_map(osmium::io::File&);


};
#endif //GRAPHS_GRAPH_HPP
