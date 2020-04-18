#include <ostream>
#include <cstdint>
#include <set>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

#include "graph.hpp"

std::ostream &operator<<(std::ostream &out, const Edge &edge) {
    out << "Edge: " << "(" << edge.from() << ", " << edge.to() << ")" << std::endl;
    return out;
}

bool Edge::operator<(const Edge &other) const {
    return _from < other._from
           ? true
           : other._from < _from
             ? false
             : _to < other._to;
}

bool Graph::add_node(uint64_t id) {
    return _nodes.emplace(id).second;
}

bool Graph::add_edge(uint64_t from, uint64_t to) {
    return _edges.emplace(from, to).second;
}

Graph Graph::from_map(osmium::io::File &input_file) {
    struct CountHandler : public osmium::handler::Handler {
        std::map<uint64_t, uint64_t> nodes_counter = std::map<uint64_t, uint64_t>();

        void way(const osmium::Way &way) noexcept {
            for (const osmium::NodeRef &nr : way.nodes()) {
                if (nodes_counter.contains(nr.positive_ref())) {
                    uint64_t val = nodes_counter.find(nr.positive_ref())->second;
                    nodes_counter.insert_or_assign(nr.positive_ref(), val + 1);
                } else {
                    nodes_counter.insert({nr.positive_ref(), 1});
                }
            }
        }
    };

    struct AddEdgesHandler : public osmium::handler::Handler {
        CountHandler &count_handler;
        Graph &graph;

        void add_edge_with_nodes(uint64_t from, uint64_t to) {
            graph.add_node(from);
            graph.add_node(to);
            graph.add_edge(from, to);
        }

        void way(const osmium::Way &way) noexcept {
            auto prev = way.nodes().begin();
            for (const osmium::NodeRef &nr : way.nodes()) {
                if ((count_handler.nodes_counter.find(nr.positive_ref())->second > 1 &&
                     nr != *way.nodes().begin()) ||
                    nr == *way.nodes().crbegin()) {
                    add_edge_with_nodes(prev->positive_ref(), nr.positive_ref());
                    prev = &nr;
                }
            }
        }
    };

    Graph graph;

    osmium::io::Reader count_reader{input_file};
    CountHandler count_handler;
    osmium::apply(count_reader, count_handler);
    count_reader.close();

    osmium::io::Reader add_edges_reader{input_file};
    AddEdgesHandler add_edges_handler{{}, count_handler, graph};
    osmium::apply(add_edges_reader, add_edges_handler);
    add_edges_reader.close();

    return graph;
}