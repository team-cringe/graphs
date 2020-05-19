#include "graph.hpp"

namespace assessment {
void nearest(int nodes, int objects) {
    (void)nodes, (void)objects;

    osmium::io::File file { "VNMap.pbf" };
    auto G = graph::import(file);
}
} // namespace assessment
