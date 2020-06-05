#include "planning.hpp"
#include <iostream>
#include "clustering.hpp"
#include "dmatrix.hpp"
#include "geojson.hpp"

using namespace graphs;

auto shortest_paths_tree(const Map& map, Building facility,
                         const Buildings& buildings) -> std::pair<Map, double> {
    auto paths = map.shortest_paths_with_trace(facility, buildings);

    auto shortest_paths_sum = std::accumulate(paths.cbegin(), paths.cend(),
                                              static_cast<double>(0),
                                              [](auto lhs, const auto& path) {
                                                  return lhs + path.distance();
                                              });

    auto tree = paths_to_map(map, paths);

    return { tree, shortest_paths_sum };
}

auto clusters(const Map& map, Buildings& houses, size_t clusters_num) {
    auto dmatrix = dmatrix_for_buildings(map, houses);
    ClusterStructure cl_st(map, Buildings(houses), move(dmatrix));

    auto features = geojson::cluster_structure_to_features(cl_st);
    auto collection = geojson::FeatureCollection();
    collection.insert(features);
    geojson::dump_to_file(collection, "dendrogram.geojson");

    auto clusters = get_k_clusters(cl_st, clusters_num);
    Colors colors = generate_colors(clusters.size());

    collection = geojson::FeatureCollection();
    double sp_sum = 0, spt_sum = 0;
    for (size_t i = 0; i < clusters.size(); ++i) {
        houses = cl_st.get_elements(clusters[i].id());
        auto[tree, shortest_paths_sum] = shortest_paths_tree(map, clusters[i].centroid(), houses);
        features = geojson::map_to_features(tree, colors[i]);
        collection.insert(features);
        std::cout << "Cluster " << clusters[i].id() << std::endl;
        std::cout << "Shortest paths sum: " << shortest_paths_sum << std::endl;
        auto shortest_paths_tree_sum = tree.weights_sum();
        std::cout << "Shortest paths tree sum: " << tree.weights_sum() << std::endl;
        sp_sum += shortest_paths_sum;
        spt_sum += shortest_paths_tree_sum;
    }
    std::cout << "Shortest paths sum for clusters: " << sp_sum << std::endl;
    std::cout << "Shortest paths tree sum for clusters: " << spt_sum << std::endl;
    geojson::dump_to_file(collection, "clusters.geojson");
}

void planning(const Map& map, int houses_num, int clusters_num) {
    (void) houses_num, (void) clusters_num;
    (void) map;

    auto houses = map.select_random_houses(houses_num);
    auto[tree, shortest_paths_sum] = shortest_paths_tree(map, map.select_random_facilities(1)[0],
                                                         houses);
    std::cout << "Shortest paths sum: " << shortest_paths_sum << std::endl;
    std::cout << "Shortest paths tree sum: " << tree.weights_sum() << std::endl;

    auto collection = geojson::map_to_geojson(tree);
    dump_to_file(collection, "shortest_path_tree.geojson");

    clusters(map, houses, clusters_num);
}
