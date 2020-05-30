#include "graph.hpp"

using namespace graph;

void nearest(int nodes, int objects) {
    auto map = import_map("NNMap.pbf");
    auto houses = map.select_random_houses(nodes);
    auto facilities = map.select_random_facilities(objects);
}
