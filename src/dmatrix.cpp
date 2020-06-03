#include "dmatrix.hpp"

using namespace graphs;

auto dmatrix_for_buildings(const Map& map,
                           const Buildings& buildings) -> DMatrix<Building> {
    DMatrix<Building> distanceMatrix;
    for (auto& b: buildings) {
        auto paths = map.shortest_paths(b, buildings);
        for (auto& path: paths) {
            auto[from, to] = path.ends();
            distanceMatrix.insert({{ from, to }, path.distance() });
        }
    }
    return distanceMatrix;
}