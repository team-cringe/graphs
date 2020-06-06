auto dijkstra(Node s) -> pair<ShortestPaths, Trail> {
    ...
    Trail trail;
    ...
    if (ds[v] + length < ds[to]) {
        set.erase({ ds[to], to });
        ds[to] = ds[v] + length;
        trail[to] = v;
        set.insert({ ds[to], to });
    }
    ...
    return { ds, trail };
}
