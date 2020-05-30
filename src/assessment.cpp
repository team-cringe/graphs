#include <iostream>

#include "graph.hpp"

void assessment(const graph::Map& map, int houses_num, int facilities_num) {
    auto houses = map.select_random_houses(houses_num);
    auto facilities = map.select_random_facilities(facilities_num);

    for (const auto& house: houses) {
        auto result = map.shortest_paths(house, facilities);
        for (const auto& path: result) {
            auto [from, to] = path.ends();
            std::cout << from.longitude() << ", " << from.latitude() << std::endl;
        }
    }
}
