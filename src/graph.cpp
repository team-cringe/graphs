#include <cstdint>
#include <set>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

#include "graph.hpp"

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