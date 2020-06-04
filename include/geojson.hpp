#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#include "nlohmann/json.hpp"

#include "map.hpp"
#include "color.hpp"
#include "clustering.hpp"

using namespace graphs;
using nlohmann::json;

json building_to_geojson_point(const Building& building, Color color = { 0.33, 0.33, 0.33 });

json path_to_geojson(const Map::TracedPath& path);

json paths_and_buildings_to_geojson(const Map::TracedPaths& paths, const Buildings& buildings);

void dump_to_file(const json& geojson, const std::string& filename = "geojson.out");

json map_to_geojson(const Map& map);

json edge_to_geojson(Location from, Location to, Color color = { 0.33, 0.33, 0.33 });

json cluster_to_geojson(const Cluster&, const ClusterStructure&,
                        Color color = { 0.33, 0.33, 0.33 });

json clusters_to_geojson(const Clusters& cls, const ClusterStructure& cl_st, Colors colors);

json cluster_structure_to_geojson(const ClusterStructure& cl_st);
#endif // GEOJSON_HPP
