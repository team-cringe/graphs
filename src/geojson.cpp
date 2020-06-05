#include "geojson.hpp"
#include <fstream>

namespace geojson {
FeatureCollection::FeatureCollection() {
    m_json = {
        { "type", "FeatureCollection" },
        { "features", {}},
    };
}
auto FeatureCollection::emplace_back(Feature& feature) {
    return m_json["features"].emplace_back(feature.json());
}
void FeatureCollection::insert(Features& features) {
    for (auto& feature: features) {
        emplace_back(feature);
    }
}
Point::Point(Location loc, Color color) {
    m_json = {
        { "type", "Feature" },
        { "properties", {
            { "marker-size", "small" },
            { "marker-color", "#" + color.hex() },
        }},
        { "geometry", {
            { "type", "Point" },
            { "coordinates", { loc.second, loc.first }}
        }}
    };
}
LineString::LineString(const Locations& locs, Color color) {
    m_json = {
        { "type", "Feature" },
        { "properties", {
            { "stroke", "#" + color.hex() },
        }},
        { "geometry", {
            { "type", "LineString" },
            { "coordinates", {}},
        }}
    };

    for (auto& loc: locs) {
        m_json["geometry"]["coordinates"]
            .emplace_back(nlohmann::json { loc.second, loc.first });
    }
}
Point building_to_point(const Building& building, Color color) {
    return Point(building.location(), color);
}
Features buildings_to_features(const Buildings& buildings, Color color) {
    return std::accumulate(buildings.begin(), buildings.end(), Features(),
                           [&](auto lhs, const auto& b) {
                               lhs.emplace_back(static_cast<Feature>(Point(b.location(), color)));
                               return lhs;
                           });
}
LineString path_to_linestring(const Map::TracedPath& path, Color color) {
    Locations locs = std::accumulate(path.path().begin(), path.path().end(), Locations(),
                                     [](auto lhs, const auto& node) {
                                         lhs.emplace_back(node.location());
                                         return lhs;
                                     });
    return LineString(locs, color);
}
Features paths_to_features(const Map::TracedPaths& paths, Color color) {
    Features linestrings =
        std::accumulate(paths.begin(), paths.end(), Features(), [&](auto lhs, const auto& path) {
            lhs.emplace_back(path_to_linestring(path, color));
            return lhs;
        });
    return linestrings;
}
Features map_to_features(const Map& map, Color color) {
    auto features = Features();

    for (auto& building: map.buildings()) {
        auto point = building_to_point(building, color);
        features.emplace_back(point);
        auto edge =
            LineString(Locations { building.location(), building.closest().location() }, color);
        features.emplace_back(edge);
    }

    for (const auto&[node, edges]: map.nodes()) {
        for (const auto& edge: edges) {
            auto edge_geojson =
                LineString(Locations { node.location(), edge.first.location() }, color);
            features.emplace_back(edge_geojson);
        }
    }

    return features;
}
FeatureCollection map_to_geojson(const Map& map, Color color) {
    auto features = map_to_features(map, color);
    auto collection = FeatureCollection();
    collection.insert(features);
    return collection;
}
Features cluster_to_features(const Cluster& cl, const ClusterStructure& cl_st, Color color) {
    auto elements = cl_st.get_elements(cl.id());
    return buildings_to_features(elements, color);
}
Features clusters_to_features(const Clusters& cls, const ClusterStructure& cl_st, Colors colors) {
    auto features = Features();
    if (colors.empty()) {
        for (auto& cl: cls) {
            auto features_new = cluster_to_features(cl, cl_st);
            features.insert(features.end(), features_new.begin(), features.end());
        }
    } else {
        for (size_t i = 0; i < cls.size(); ++i) {
            auto features_new = cluster_to_features(cls[i], cl_st, colors[i]);
            features.insert(features.end(), features_new.begin(), features.end());
        }
    }
    return features;
}
void dump_to_file(FeatureCollection& collection, const std::string& filename) {
    std::ofstream outfile(filename);
    outfile << collection.json().dump(4);
}
Features cluster_structure_to_features(const ClusterStructure& cl_st) {
    auto features = Features();

    for (auto& cluster: cl_st.clusters()) {
        if (cluster.left()) {
            features.emplace_back(LineString(Locations { cluster.left()->centroid().location(),
                                                         cluster.centroid().location() }));
            features.emplace_back(LineString(Locations { cluster.right()->centroid().location(),
                                                         cluster.centroid().location() }));
        }
    }

    return features;
}
Features paths_and_buildings_to_features(const Map::TracedPaths& paths,
                                         const Buildings& buildings, Color color) {
    auto features = Features();
    auto features_new = paths_to_features(paths, color);
    features.insert(features.end(), features_new.begin(), features_new.end());
    features_new = buildings_to_features(buildings, color);
    features.insert(features.end(), features_new.begin(), features_new.end());
    return features;
}
} // namespace geojson
