auto shortest_paths_with_trace(Building from, 
    const Buildings& to) -> TracedPaths {
    auto[distances, trail] = 
        m_graph.dijkstra(from.closest());
    TracedPaths result {};

    for (const auto& building: to) {
        auto node_to = building.closest(),
             node_from = from.closest();
        auto distance = distances.at(node_to);

        // Reconstruct path.
        std::vector<Node> path;
        for (auto v = node_to; v != from.closest();
            v = trail[v]) {
            path.push_back(v);
        }
        path.push_back(node_from);
        reverse(path.begin(), path.end());

        // Build path in place.
        result.emplace_back(from, building, path, 
            distance);
    }

    return result;
}