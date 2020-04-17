#include "graph.hpp"
#include <cstdint>
#include <set>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

const Node& Graph::add_node(uint64_t id) {
    return *_nodes.emplace(id).first;
}

void Graph::add_edge(Node& from, Node& to) {
    _edges.emplace(from, to);
}


const set<Node>& Graph::nodes() const {
    return _nodes;
}

const set<Edge>& Graph::edges() const {
    return _edges;
}

Graph Graph::from_map(osmium::io::File& input_file) {
    struct CountHandler: public osmium::handler::Handler {
        std::map<uint64_t, uint64_t> nodesCounter = std::map<uint64_t, uint64_t>();
//            Graph& graph;
        void way(const osmium::Way& way) noexcept {
            for (const osmium::NodeRef& nr : way.nodes()) {
                if (nodesCounter.contains(nr.positive_ref())) {
                    uint64_t val = nodesCounter.find(nr.positive_ref())->second;
                    nodesCounter.insert_or_assign(nr.positive_ref(), val + 1);
                } else {
                    nodesCounter.insert({nr.positive_ref(), 1});
                }
            }
        }
    };

    struct AddEdgesHandler: public osmium::handler::Handler {
        CountHandler& count_handler;
        Graph& graph;
        void way(const osmium::Way& way) noexcept {
            auto prev = way.nodes().begin();
            for (const osmium::NodeRef &nr : way.nodes()) {
                if (count_handler.nodesCounter.find(nr.positive_ref())->second > 1 &&
                    nr != *way.nodes().begin()) {
                    auto from = graph.add_node(prev->positive_ref());
                    auto to = graph.add_node(nr.positive_ref());
                    graph.add_edge(from, to);
                    prev = &nr;
                } else if (nr == *way.nodes().crbegin()) {
                    auto from = graph.add_node(prev->positive_ref());
                    auto to = graph.add_node(nr.positive_ref());
                    graph.add_edge(from, to);
                    prev = &nr;
                }
            }
        }
    };

    Graph graph;

    osmium::io::Reader count_reader{input_file};
    osmium::io::Reader add_edges_reader{input_file};


    CountHandler count_handler;
    osmium::apply(count_reader, count_handler);
    count_reader.close();

    AddEdgesHandler add_edges_handler{{}, count_handler, graph};
    osmium::apply(add_edges_reader, add_edges_handler);
    add_edges_reader.close();

    return graph;
}