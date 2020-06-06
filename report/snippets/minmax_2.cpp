    auto result = min_element(
        from.cbegin(), from.cend(),
        [&](const auto& lhs, const auto& rhs) {
            return furthest[lhs] < furthest[rhs];
        });
    return result;
}
