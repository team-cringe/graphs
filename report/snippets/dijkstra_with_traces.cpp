auto dijkstra(Node s) -> pair<ShortestPaths, Trail> {
    std::unordered_map<Node, Distance> ds;
    std::set<pair<Distance, Node>> set;
    std::unordered_map<Node, Node> previous;

    for (const auto&[v, _]: nodes()) { ds[v] = INF; }

    ds[s] = 0;
    set.insert({ ds[s], s });
    while (!set.empty()) {
        auto[_, v] = *set.begin();
        set.erase(set.begin());
        for (const auto& u: nodes().at(v)) {
            auto[to, length] = u;
            if (ds[v] + length < ds[to]) {
                set.erase({ ds[to], to });
                ds[to] = ds[v] + length;
                previous[to] = v;
                set.insert({ ds[to], to });
            }
        }
    }

    return { ds, previous };
}