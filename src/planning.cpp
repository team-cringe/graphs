#include "planning.hpp"
#include <iostream>

using namespace graphs;

auto shortest_paths_tree(const Map& map, Building facility, const Buildings& buildings) -> Map {
    auto paths = map.shortest_paths_with_trace(facility, buildings);

    auto shortest_paths_sum = std::accumulate(paths.cbegin(), paths.cend(),
                                              static_cast<double>(0),
                                              [](auto lhs, const auto& path) {
                                                  return lhs + path.distance();
                                              });

    auto tree = paths_to_map(map, paths);
    auto shortest_paths_tree_sum = tree.weights_sum();
    std::cout << "Shortest paths sum: " << shortest_paths_sum << std::endl;
    std::cout << "Shortest paths tree sum: " << shortest_paths_tree_sum << std::endl;

    return tree;
}

void planning(const Map& map, int houses_num, int facilities_num) {
    (void) houses_num, (void) facilities_num;
    (void) map;
}
