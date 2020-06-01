#ifndef DISTANCE_MATRIX_HPP
#define DISTANCE_MATRIX_HPP

#include "distance_matrix.hpp"
#include "graph.hpp"

using namespace graph;

template <typename T>
using DMatrix = std::unordered_map<std::pair<T, T>, double>;

namespace std {
template<>
struct hash<std::pair<Building, Building>> {
    size_t operator()(const std::pair<Building, Building>& p) const {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        auto[b1, b2] = p;
        hash_combine(seed, hash_value((b1.id())));
        hash_combine(seed, hash_value((b2.id())));
        return seed;
    }
};
}

/**
 * Factory function for distance matrix of buildings.
 * Calculates distances with dijkstra and stores them in DMatrix.
 */
auto dmatrix_for_buildings(const Map& map, const Buildings& buildings) -> DMatrix<Building>;


#endif //DISTANCE_MATRIX_HPP
