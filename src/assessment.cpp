#include "graph.hpp"

using namespace graph;

namespace assessment {
void nearest(int nodes, int objects) {
    auto map = graph::import_map("NNMap.pbf");
    auto houses = map.select_random_houses(nodes);
    auto facilities = map.select_random_facilities(objects);

    for (const auto&[house, _]: houses) {
        auto result = map.shortest_paths(house, facilities);
    }
}
} // namespace assessment
