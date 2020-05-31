#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#include "nlohmann/json.hpp"
#include "graph.hpp"

using namespace graph;
using nlohmann::json;

json building_to_geojson_point(const Building& building);

json path_to_geojson(const Map::TracedPath& path);

json paths_and_buildings_to_geojson(const Map::TracedPaths& paths, const Buildings& buildings);

void dump_to_file(const json& geojson, std::string filename = "geojson.out");

#endif // GEOJSON_HPP
