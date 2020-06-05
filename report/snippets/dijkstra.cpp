auto dijkstra(Node s) -> ShortestPaths {
    unordered_map<Node, Distance> ds;
    set<pair<Distance, Node>> set;

    for (const auto&[v, _]: nodes()) { ds[v] = INF; }
    ds[s] = 0;
    set.insert({ ds[s], s });
    
    while (!set.empty()) {
        auto [v, _] = set.begin();
        set.erase(set.begin());
        for (auto& u: nodes()[v]) {
            auto [to, length] = u;
            if (ds[v] + length < ds[to]) {
                set.erase({ ds[to], to });
                ds[to] = ds[v] + length;
                set.insert({ ds[to], to });
            }
        }
    }

    return ds;
}
