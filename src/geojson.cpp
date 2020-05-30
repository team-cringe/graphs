#include "nlohmann/json.hpp"
#include "graph.hpp"

#include "geojson.hpp"
#include "fstream"

using nlohmann::json;


json building_to_geojson_point(const graph::Building& building) {
    return json {
        {"type", "Feature"},
        {"properties", {}},
        {"geometry", {
            {"type", "Point"},
            {"coordinates", {building.longitude(), building.latitude()}}
        }}
    };
}

json path_to_geojson(const graph::Map::Path& path) {
    auto [from, to] = path.ends();
    json way = {
        {"type", "Feature"},
        {"properties", {}},
        {"geometry", {
            {"type", "LineString"},
            {"coordinates", {
                {from.longitude(), from.latitude()}
            }}
        }}
    };

    for (const auto& node: path.path()) {
        way["geometry"]["coordinates"].push_back({node.longitude(), node.latitude()});
    }

    way["geometry"]["coordinates"].push_back({to.longitude(), to.latitude()});

    return way;
}

json paths_and_buildings_to_geojson(const Map::Paths& paths, const Buildings& buildings) {
    json geojson = {
        {"type", "FeatureCollection"},
        {"features", {}},
    };

    for (auto& building: buildings) {
        auto point = building_to_geojson_point(building);
        geojson["features"].push_back(point);
    }

    for (const auto& path: paths) {
        auto path_geojson = path_to_geojson(path);
        geojson["features"].push_back(path_geojson);
    }

    return geojson;
}

void dump_to_file(const json& geojson, std::string filename) {
    std::ofstream outfile(filename);
    outfile << geojson.dump(4);
}