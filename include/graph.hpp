#ifndef GRAPHS_GRAPH_HPP
#define GRAPHS_GRAPH_HPP

#include <ostream>
#include <vector>
#include <osmium/visitor.hpp>
#include <queue>
#include <stack>

using AdjacencyList = std::vector<uint64_t>;

class Node {
public:
    enum Color { white, gray, black };
private:
    uint64_t _id_osm;
    mutable AdjacencyList _neighbors;
    //bfs and dfs
    Color _color;
    Node* _parent;

    uint64_t _d;
    uint64_t _f;

    static uint64_t _time;

public:

    explicit Node(uint64_t id_osm)
        : _id_osm(id_osm)
        , _neighbors(AdjacencyList())
        , _color(white)
        , _parent(nullptr)
        , _d(0)
        , _f(0) {}

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

    Color color() const {
        return _color;
    }

    friend class Graph;
};

class Graph {
    std::vector<Node> _nodes;
public:
    explicit Graph()
        : _nodes(std::vector<Node>()) {}

    explicit Graph(std::vector<Node>&& nodes)
        : _nodes(nodes) {}

    bool add_node(uint64_t id_osm);

    bool add_edge(uint64_t from, uint64_t to);

    [[nodiscard]]
    const std::vector<Node>& nodes() const {
        return _nodes;
    }

    static Graph from_osm(osmium::io::File&);

    void bfs(uint64_t start_id);

    void dfs(uint64_t start_id);
};

#endif //GRAPHS_GRAPH_HPP
