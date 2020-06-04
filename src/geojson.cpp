#include "geojson.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

using nlohmann::json;

json building_to_geojson_point(const graphs::Building& building, Color color) {
    return json {
        { "type", "Feature" },
        { "properties", {
            { "marker-size", "small" },
            { "marker-color", "#" + color.hex() },
        }},
        { "geometry", {
            { "type", "Point" },
            { "coordinates", { building.longitude(), building.latitude() }}
        }}
    };
}

json path_to_geojson(const graphs::Map::TracedPath& path) {
    auto[from, to] = path.ends();
    json way = {
        { "type", "Feature" },
        { "properties", {}},
        { "geometry", {
            { "type", "LineString" },
            { "coordinates", {
                { from.longitude(), from.latitude() }
            }}
        }}
    };

    for (const auto& node: path.path()) {
        way["geometry"]["coordinates"].push_back({ node.longitude(), node.latitude() });
    }

    way["geometry"]["coordinates"].push_back({ to.longitude(), to.latitude() });

    return way;
}

json paths_and_buildings_to_geojson(const Map::TracedPaths& paths, const Buildings& buildings) {
    json geojson = {
        { "type", "FeatureCollection" },
        { "features", {}},
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

json edge_to_geojson(Node from, Node to, Color color = { 0.33, 0.33, 0.33 }) {
    return {
        { "type", "Feature" },
        { "properties", {
            { "stroke", "#" + color.hex() }
        }},
        { "geometry", {
            { "type", "LineString" },
            { "coordinates", {
                { from.longitude(), from.latitude() },
                { to.longitude(), to.latitude() },
            }}
        }}
    };
}

json edge_to_geojson(Building from, Node to, Color color = { 0.33, 0.33, 0.33 }) {
    return {
        { "type", "Feature" },
        { "properties", {
            { "stroke", "#" + color.hex() }
        }},
        { "geometry", {
            { "type", "LineString" },
            { "coordinates", {
                { from.longitude(), from.latitude() },
                { to.longitude(), to.latitude() },
            }}
        }}
    };
}

json edge_to_geojson(Location from, Location to, Color color) {
    return {
        { "type", "Feature" },
        { "properties", {
            { "stroke", "#" + color.hex() }
        }},
        { "geometry", {
            { "type", "LineString" },
            { "coordinates", {
                from,
                to,
            }}
        }}
    };
}

void dump_to_file(const json& geojson, const std::string& filename) {
    std::ofstream outfile(filename);
    outfile << geojson.dump(4);
}

json map_to_geojson(const Map& map) {
    json geojson = {
        { "type", "FeatureCollection" },
        { "features", {}},
    };

    for (auto& building: map.buildings()) {
        auto point = building_to_geojson_point(building);
        geojson["features"].push_back(point);

        auto edge = edge_to_geojson(building, building.closest());
        geojson["features"].push_back(edge);
    }

    for (const auto&[node, edges]: map.nodes()) {
        for (const auto& edge: edges) {
            auto edge_geojson = edge_to_geojson(node, edge.first);
            geojson["features"].push_back(edge_geojson);
        }
    }

    return geojson;
}