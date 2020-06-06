auto minmax(const Map& map, const Buildings& from,
                            const Buildings& to) {
    unordered_map<Building, Distance> furthest {};
    for (auto& f: from) {
        auto paths = map.shortest_paths(f, to);
        furthest[f] = max_element(
            paths.cbegin(), paths.cend(),
            [](const auto& lhs, const auto& rhs) {
                return
                lhs.distance() > rhs.distance() &&
                lhs.distance() < INF;
            }).distance();
    }
