#include <cstdint>
#include <set>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

#include "graph.hpp"

uint64_t Node::_time = 0;

bool Graph::add_node(uint64_t id_osm) {
    _nodes.emplace_back(id_osm);
    return true;
}

bool Graph::add_edge(uint64_t from, uint64_t to) {
    if (from >= _nodes.size() or to >= _nodes.size()) {
        return false;
    }
    for (uint64_t edge: _nodes[from].neighbors()) {
        if (edge == to) {
            return false;
        }
    }
    _nodes[from]._neighbors.emplace_back(to);
    return true;
}

Graph Graph::from_osm(osmium::io::File& input_file) {
    struct CountHandler: public osmium::handler::Handler {
        // stores count of ways it's used in and index in the vector;
        std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>>
            nodes_info = std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>>();
        // stores nodes
        std::vector<Node> nodes;

        void way(const osmium::Way& way) noexcept {
            for (const osmium::NodeRef& nr : way.nodes()) {
                //if node is already in the nodes list we increment counter and save it
                //else we add node to the list with a counter equal to one
                if (nodes_info.contains(nr.positive_ref())) {
                    std::pair<uint64_t, uint64_t> info = nodes_info.find(nr.positive_ref())->second;
                    info.first += 1;
                    nodes_info.insert_or_assign(nr.positive_ref(), info);
                } else {
                    nodes_info.insert({ nr.positive_ref(), { 1, nodes.size() }});
                    nodes.emplace_back(nr.positive_ref());
                }
            }
        }
    };

    struct AddEdgesHandler: public osmium::handler::Handler {
        std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>> nodes_info;
        Graph& graph;

        void way(const osmium::Way& way) noexcept {
            auto prev = way.nodes().begin();
            for (const osmium::NodeRef& nr : way.nodes()) {
                auto info_to = nodes_info.find(nr.positive_ref())->second;
                if ((info_to.first > 1 and
                     nr != *way.nodes().begin()) or
                    nr == *way.nodes().crbegin()) {
                    auto info_from = nodes_info.find(prev->positive_ref())->second;
                    uint64_t from = info_from.second;
                    uint64_t to = info_to.second;
                    graph.add_edge(from, to);
                    prev = &nr;
                }
            }
        }
    };

    osmium::io::Reader count_reader { input_file };
    CountHandler count_handler;
    osmium::apply(count_reader, count_handler);
    count_reader.close();

    Graph graph(std::move(count_handler.nodes));

    osmium::io::Reader add_edges_reader { input_file };
    AddEdgesHandler add_edges_handler {{}, std::move(count_handler.nodes_info), graph };
    osmium::apply(add_edges_reader, add_edges_handler);
    add_edges_reader.close();

    return graph;
}

void Graph::bfs(uint64_t start_id) {
    for (auto& node : _nodes) {
        node._color = Node::Color::white;
        node._parent = nullptr;
        node._d = 0;
    }
    _nodes[start_id]._color = Node::Color::gray;
    std::queue<Node*> Q = std::queue<Node*>();
    Q.push(&_nodes[start_id]);

    while (not Q.empty()) {
        Node* u = Q.front();
        Q.pop();
        for (auto v_id: u->_neighbors) {
            auto& v = _nodes[v_id];
            if (v._color == Node::Color::white) {
                v._color = Node::Color::gray;
                v._d = u->_d + 1;
                v._parent = u;
                Q.push(&v);
            }
        }
        u->_color = Node::Color::black;
    }
}

void Graph::dfs(uint64_t start_id) {
    for (auto& u: _nodes) {
        u._color = Node::Color::white;
        u._parent = nullptr;
    }
    Node::_time = 0;

    std::stack<uint64_t> S;
    Node* v_curr = &_nodes[start_id];
    v_curr->_d = Node::_time;
    S.push(0);

    while (not S.empty()) {
        // if not all nodes are visited
        while (S.top() < v_curr->_neighbors.size()) {
            auto u = &_nodes[v_curr->_neighbors[S.top()]];
            if (u->_color != Node::Color::white) {
                ++S.top();
                continue;
            }
            u->_d = ++Node::_time;
            u->_color = Node::Color::gray;
            u->_parent = v_curr;
            v_curr = u;
            S.push(0);
        }
        v_curr->_color = Node::Color::black;
        v_curr->_f = ++Node::_time;
        v_curr = v_curr->_parent;
        S.pop();
    }
}