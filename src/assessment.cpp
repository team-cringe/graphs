#include "graph.hpp"

#include <iostream>

namespace assessment {
void nearest(int nodes, int objects) {
    (void)nodes;

    osmium::io::File file { "VNMap.pbf" };
    auto map = graph::import_map(file);

    auto facilities = map.select_random_facilities(objects);
    for (const auto& f: facilities) {
        std::cout << f.second << std::endl;
    }
}
} // namespace assessment
