#ifndef GRAPHS_MAP_HPP
#define GRAPHS_MAP_HPP

#include "utils.hpp"
#include "building.hpp"
#include "graph.hpp"

namespace fs = std::filesystem;

namespace graphs {
struct Map {
    Map() = default;

    Map(Buildings buildings, Graph graph)
        : m_buildings(std::move(buildings))
        , m_graph(std::move(graph)) {};

    /**
     * Pair of Buildings with the Distance between them.
     */
    struct Path {
        Path(Building from, Building to, Distance distance)
            : m_from(from)
            , m_to(to)
            , m_distance(distance) {};

        [[nodiscard]] auto ends() const -> std::pair<Building, Building> {
            return { m_from, m_to };
        }
        [[nodiscard]] Distance distance() const { return m_distance; }

    private:
        Building m_from, m_to;
        Distance m_distance;
    };

    struct TracedPath: public Path {
        TracedPath(Building from, Building to, Nodes path, Distance distance)
            : Path(from, to, distance)
            , m_trace(std::move(path)) {};

        [[nodiscard]] const Nodes& path() const { return m_trace; }

    private:
        Nodes m_trace;
    };

    using Paths = std::vector<Path>;
    using TracedPaths = std::vector<TracedPath>;

    /**
     * Select buildings by applying functor to each.
     *
     * @param functor [](const Building&) { return __; }
     * @return Vector of corresponding nodes.
     */
    template<typename F> [[maybe_unused]]
    auto select_buildings(F&& functor) const -> Buildings;

    /**
     * Select N buildings and apply functor to each.
     *
     * @param num Number of building to select.
     * @param functor [](const Buildings&) { return __; }
     * @return Vector of corresponding nodes.
     */
    template<typename F>
    auto select_random_buildings(size_t num, F&& functor) const -> Buildings;
    auto select_random_facilities(size_t num) const -> Buildings;
    auto select_random_houses(size_t num) const -> Buildings;

    /**
     * Get shortest paths from Node to all other Nodes specified.
     *
     * @return Mapping from Nodes to the corresponding paths from the given Node.
     */
    auto shortest_paths_with_trace(Building from, const Buildings& to) const -> TracedPaths;
    auto shortest_paths(Building from, const Buildings& to) const -> Paths;

    /**
     * Summarize all edges' weights.
     */
    auto weights_sum() const -> long double;

    bool serialize(const fs::path& filename) const;
    bool deserialize(const fs::path& filename);

    const auto& buildings() const { return m_buildings; }
    const auto& nodes() const { return m_graph.nodes(); }

private:
    Buildings m_buildings {};
    Graph m_graph {};
};

using Maps = std::vector<Map>;

/**
 * Converts paths to the new Map.
 */
auto paths_to_map(const Map& map, const Map::TracedPaths& paths) -> Map;

/**
 * Constructs routing graph based on provided PBF file with OSM geodata.
 *
 * @param file PBF file.
 * @param recache Object should be constructed from scratch and dumped.
 * @return Constructed routing graph and the list of buildings.
 */
auto import_map_from_pbf(const fs::path& filename, bool recache) -> std::optional<Map>;

/**
 * Construct routing graph based on CSV file with adjacency matrix.
 *
 * @param filename CSV file.
 * @param recache Object should be constructed from scratch and dumped.
 * @return Constructed routing graph.
 */
auto import_map_from_csv(const fs::path& filename, bool recache) -> std::optional<Map>;
} // namespace graphs

#endif // GRAPHS_MAP_HPP
