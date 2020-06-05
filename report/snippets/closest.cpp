void way(Way& way) noexcept {
    const auto location = barycenter(way);
    auto closest = min_element(routes.nodes(),
        [&](const auto& lhs, const auto& rhs) {
            return
                haversine(lhs, location) <
                haversine(rhs, location);
        });
    auto b = Building { way, location, closest };
    buildings.push(b);
}
