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
                { from.second, from.first },
                { to.second, to.first },
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

json cluster_to_geojson(const Cluster& cl, const ClusterStructure& cl_st, Color color) {
    auto buildings = cl_st.get_elements(cl.id());

    std::cout << cl.size() << "from lol" << buildings.size() << std::endl;

    json geojson = {};
    for (auto& b: buildings) {
        geojson.emplace_back(building_to_geojson_point(b, color));
    }

    return geojson;
}

json clusters_to_geojson(const Clusters& cls, const ClusterStructure& cl_st, Colors colors) {
    json geojson = {
        { "type", "FeatureCollection" },
        { "features", {}},
    };

    for (size_t i = 0; i < cls.size(); ++i) {
        json buildings = cluster_to_geojson(cls[i], cl_st, colors[i]);
        std::cout << cls[i].size() << "to " << buildings.size() << std::endl;

        for (auto& b: buildings) {
            geojson["features"].emplace_back(b);
        }
    }

    return geojson;
}

json cluster_structure_to_geojson(const ClusterStructure& cl_st) {
    json geojson = {
        { "type", "FeatureCollection" },
        { "features", {}},
    };

    for (auto& cluster: cl_st.clusters()) {
        if (cluster.left()) {
            geojson["features"]
                .emplace_back(edge_to_geojson(cluster.left()->centroid().location(),
                                              cluster.centroid().location()));
            geojson["features"]
                .emplace_back(edge_to_geojson(cluster.right()->centroid().location(),
                                              cluster.centroid().location()));
        }
    }

    return geojson;
}