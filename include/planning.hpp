#ifndef PLANNING_HPP
#define PLANNING_HPP

#include "map.hpp"

using namespace graphs;

void planning(const Map& map, int houses_num, int facilities_num);

auto shortest_paths_tree(const Map& map, Building facility, const Buildings& buildings) -> Map;

#endif // PLANNING_HPP
