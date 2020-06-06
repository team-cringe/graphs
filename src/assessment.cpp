#include "assessment.hpp"

using namespace graphs;

constexpr auto INF = std::numeric_limits<double>::max();

/**
 * For each Node:
 *   define closest Facility (to, from, to-and-back).
 */
auto closest(const Map& map, const Buildings& from, const Buildings& to) -> Map::Paths {
    Map::Paths result;
    for (const auto& f: from) {
        const auto paths = map.shortest_paths(f, to);
        const auto closest = std::min_element(paths.cbegin(), paths.cend(),
                                              [](const auto& a, const auto& b) {
                                                  return a.distance() < b.distance();
                                              });
        if (closest->distance() == INF) { continue; }
        result.push_back(*closest);
    }
    return result;
}

/**
 * For each Node:
 *   define Buildings no further than X meters.
 */
auto range(const Map& map, const Buildings& from, const Buildings& to, uint64_t x) -> Map::Paths {
    Map::Paths result;
    for (const auto& f: from) {
        const auto paths = map.shortest_paths(f, to);
        for (const auto& path: paths) {
            if (path.distance() <= x) {
                result.push_back(path);
            }
        }
    }
    return result;
}

/**
 * Define Building that has minimal distance between it and the furthest Node.
 */
auto minmax(const Map& map, const Buildings& from, const Buildings& to) -> Building {
    std::unordered_map<Building, Distance> furthest {};
    for (const auto& f: from) {
        const auto paths = map.shortest_paths(f, to);
        furthest[f] = std::max_element(paths.cbegin(), paths.cend(),
                                       [](const auto& lhs, const auto& rhs) {
                                           return lhs.distance() > rhs.distance() &&
                                                  lhs.distance() < INF;
                                       })->distance();
    }
    const auto result = std::min_element(from.cbegin(), from.cend(),
                                         [&](const auto& lhs, const auto& rhs) {
                                             return furthest[lhs] < furthest[rhs];
                                         });
    return *result;
}

/**
 * Define Buildings that has minimal sum of the shortest paths (median).
 */
auto median(const Map& map, const Buildings& from, const Buildings& to) -> Building {
    std::unordered_map<Building, Distance> sum {};
    for (const auto& f: from) {
        const auto paths = map.shortest_paths(f, to);
        sum[f] = std::accumulate(paths.cbegin(), paths.cend(), static_cast<double>(0),
                                 [](double s, const auto& path) {
                                     return path.distance() < INF ? s += path.distance() : 0;
                                 });
    }
    const auto result = std::min_element(from.cbegin(), from.cend(),
                                         [&](const auto& lhs, const auto& rhs) {
                                             return sum[lhs] < sum[rhs];
                                         });
    return *result;
}

void assessment(const Map& map, int houses_num, int facilities_num) {
    constexpr auto x = 800;

    std::ofstream report { "report.txt" };

    auto houses = map.select_random_houses(houses_num);
    auto facilities = map.select_random_facilities(facilities_num);

    {
        auto ch2f = closest(map, houses, facilities);
        auto cf2h = closest(map, facilities, houses);

        report << "Closest house -> facility:\n";
        for (const auto& path: ch2f) {
            auto[from, to] = path.ends();
            report << from.id() << "---" << to.id() << "---" << path.distance() << "\n";
        }

        report << "Closest facility -> house:\n";
        for (const auto& path: cf2h) {
            auto[from, to] = path.ends();
            report << from.id() << "---" << to.id() << "---" << path.distance() << "\n";
        }
    }

    {
        auto rh2f = range(map, houses, facilities, x);
        auto rf2h = range(map, facilities, houses, x);

        report << "In range house -> facility:\n";
        for (const auto& path: rh2f) {
            auto[from, to] = path.ends();
            report << from.id() << "---" << to.id() << "---" << path.distance() << "\n";
        }

        report << "In range facility -> house:\n";
        for (const auto& path: rf2h) {
            auto[from, to] = path.ends();
            report << from.id() << "---" << to.id() << "---" << path.distance() << "\n";
        }
    }

    {
        auto mmh2f = minmax(map, houses, facilities);
        auto mmf2h = minmax(map, facilities, houses);

        report << "Minmax house -> facility:\n";
        report << mmh2f.id() << "\n";

        report << "Minmax facility -> house:\n";
        report << mmf2h.id() << "\n";
    }

    {
        auto m = median(map, houses, facilities);

        report << "Median:\n";
        report << m.id() << "\n";
    }
}
