Map paths_to_map(Map& map, Map::TracedPaths& paths) {
    std::unordered_set<Building> set;
    Buildings buildings;
    Graph routes;

    for (const auto& path: paths) {
        auto[from, to] = path.ends();
        set.insert(from);
        set.insert(to);

        auto pred = *path.path().begin();
        for (const auto curr: path.path()) {
            if (curr == pred) { continue; }
            //weight of edge from pred to curr
            auto w = map.nodes().find(pred)
                ->second.find(curr)->second;
            routes.add_edge_one_way({ pred, curr },
                w);
            pred = curr;
        }
    }

    buildings.insert(buildings.end(), set.begin(),
        set.end());

    return Map { buildings, routes };
}