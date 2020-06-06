auto shortest_paths(Building from, Buildings& to) {
    auto[distances, trail] = G.dijkstra(from);
    TracedPaths result;

    for (const auto& to: tos) {
        auto distance = distances[to];

        // Reconstruct path.
        vector<Node> path;
        for (auto v = node; 
                  v != from; v = trail[v]) {
            path.push(v);
        }
        path.push(from);
        reverse(path.begin(), path.end());

        result.push({ from, to, path, distance });
    }

    return result;
}
