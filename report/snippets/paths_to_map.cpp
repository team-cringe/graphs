auto paths_to_map(Map& map, TracedPaths& paths) {
    unordered_set<Building> set;
    Buildings buildings;
    Graph routes;

    for (auto& path: paths) {
        auto[from, to] = path.ends();
        set.insert(from);
        set.insert(to);
        auto pred = path.begin();

        for (auto curr: path) {
            if (curr == pred) { continue; }
            // Weight of the edge from pred to curr.
            auto w = map.find(pred).find(curr);
            routes.add_edge({ pred, curr }, w);
            pred = curr;
        }
    }

    buildings.insert(buildings.end(), 
                     set.begin(), 
                     set.end());

    return Map { buildings, routes };
}
