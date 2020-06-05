#include "graph.hpp"

#include <unordered_map>
#include <filesystem>
#include <limits>

#include <boost/serialization/unordered_map.hpp>

#include "utils.hpp"

namespace graphs {
bool Graph::serialize(const fs::path& filename) const {
    auto cname = filename;
    return ::graphs::serialize(cname.concat("-gph.dmp"), m_data);
}

bool Graph::deserialize(const fs::path& filename) {
    auto cname = filename;
    cname.concat("-gph.dmp");
    if (!std::filesystem::exists(cname)) { return false; }
    return ::graphs::deserialize(cname, m_data);
}
} // namespace graphs

namespace graphs {
bool Graph::add_edge_one_way(Edge&& e, Distance d) noexcept {
    auto[from, to] = e;
    if (from == to) { return false; }
    return m_data[from].insert({ to, d }).second;
}

bool Graph::add_edge_two_way(Edge&& e, Distance d) noexcept {
    auto[from, to] = e;
    if (from == to) { return false; }
    return m_data[from].insert({ to, d }).second && m_data[to].insert({ from, d }).second;
}

auto Graph::dijkstra(Node s) const -> std::pair<ShortestPaths, Trail> {
    constexpr auto INF = std::numeric_limits<double>::max();

    std::unordered_map<Node, Distance> distances;
    std::set<std::pair<Distance, Node>> set;
    std::unordered_map<Node, Node> previous;

    for (const auto&[node, _]: nodes()) { distances[node] = INF; }

    distances[s] = 0;
    set.insert({ distances[s], s });
    while (!set.empty()) {
        auto[_, v] = *set.begin();
        set.erase(set.begin());
        for (const auto& u: nodes().at(v)) {
            auto[to, length] = u;
            if (distances[v] + length < distances[to]) {
                set.erase({ distances[to], to });
                distances[to] = distances[v] + length;
                previous[to] = v;
                set.insert({ distances[to], to });
            }
        }
    }

    return { distances, previous };
}
} // namespace graph
