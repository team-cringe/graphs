#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#include "nlohmann/json.hpp"

#include "map.hpp"
#include "color.hpp"
#include "clustering.hpp"

using namespace graphs;

namespace geojson {

struct Json {
    auto json() { return m_json; }
protected:
    nlohmann::json m_json;
};
using Feature = Json;
using Features = std::vector<Json>;

struct FeatureCollection: Json {
    FeatureCollection();
    auto emplace_back(Feature& feature);
    void insert(Features& features);
};

struct Point: Json {
    Point() = delete;
    explicit Point(Location loc, Color color = Color::gray);
};

struct LineString: Json {
    LineString() = delete;
    explicit LineString(const Locations& locs, Color color = Color::gray);
};

Point building_to_point(const Building& building, Color color = Color::gray);
Features buildings_to_features(const Buildings& buildings, Color color = Color::gray);

LineString path_to_linestring(const Map::TracedPath& path, Color color = Color::gray);
Features paths_to_features(const Map::TracedPaths& paths, Color color = Color::gray);

Features paths_and_buildings_to_features(const Map::TracedPaths& paths,
                                         const Buildings& buildings,
                                         Color color = Color::gray);

Features map_to_features(const Map& map, Color color = Color::gray);
FeatureCollection map_to_geojson(const Map& map, Color color = Color::gray);

Features cluster_to_features(const Cluster& cl, const ClusterStructure& cl_st,
                             Color color = Color::gray);
Features clusters_to_features(const Clusters& cls, const ClusterStructure& cl_st,
                              Colors colors = Colors());
Features cluster_structure_to_features(const ClusterStructure& cl_st);
void dump_to_file(FeatureCollection& collection, const std::string& filename = "output.geojson");
} // namespace geojson
#endif // GEOJSON_HPP
