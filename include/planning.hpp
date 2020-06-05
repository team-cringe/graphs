#ifndef PLANNING_HPP
#define PLANNING_HPP

#include "map.hpp"

using namespace graphs;

void planning(const Map& map, int houses_num, int clusters_num);

auto shortest_paths_tree(const Map& map, Building facility,
                         const Buildings& buildings) -> std::pair<Map, double>;

#endif // PLANNING_HPP
