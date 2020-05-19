#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include <boost/functional/hash.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

namespace graph {
using Distance = double;
using Angle = long double;
using Position = std::pair<Angle, Angle>;

using Node = std::uint64_t;
using Edge = std::pair<Node, Node>;

struct Graph {
private:
    using OutgoingEdges = std::unordered_map<Node, Distance>;
    using AdjacencyList = std::unordered_map<Node, OutgoingEdges>;

public:
    void add_edge(Edge&& e, Distance d = 0) {
        auto[from, to] = e;
        if (from == to) { return; }
        data[from].insert({ to, d });
        data[to].insert({ from, d });
    }

    auto nodes() {
        return data;
    }

private:
    AdjacencyList data {};
};

/**
 * Factory method for position.
 */
auto make_pos(const osmium::NodeRef& node) -> Position;

/**
 * Constructs routing graph based on provided OSM geodata.
 *
 * @param file File with geographic data.
 * @return Constructed routing graph.
 */
auto import(osmium::io::File& file) -> Graph;

/**
 * Determines the great-circle distance between two points given their longitudes and latitudes.
 *
 * @param x, y OSM nodes with corresponding coordinates.
 * @return Distance between nodes.
 */
auto haversine(const osmium::NodeRef& x, const osmium::NodeRef& y) -> Distance;

/**
 * Determines the geographical center of a building consisting of ambient nodes.
 *
 * @param nodes List of nodes of an OSM way.
 * @return Geocenter described by a pair of latitude and longitude respectively.
 */
auto barycenter(const osmium::WayNodeList& nodes) -> Position;
} // namespace graph

#endif // GRAPH_HPP
