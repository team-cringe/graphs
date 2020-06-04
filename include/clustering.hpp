#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include <unordered_set>

#include <boost/numeric/ublas/matrix.hpp>

#include "dmatrix.hpp"

using namespace graphs;

auto centroid(const Map& map, Locations locs) -> Building;

auto find_nearest_building(const Map& map, Location loc) -> Building;

struct Cluster {
    Cluster() = delete;

    explicit Cluster(size_t first, Building b)
        : m_left(nullptr)
        , m_right(nullptr)
        , m_size(1)
        , m_first(first)
        , m_last(first)
        , m_id(overall_clusters_num++)
        , m_center(std::move(b)) {}

    explicit Cluster(Cluster& c1, Cluster& c2, Building b)
        : m_left(&c1)
        , m_right(&c2)
        , m_size(c1.m_size + c2.m_size)
        , m_first(c1.m_first)
        , m_last(c2.m_last)
        , m_id(overall_clusters_num++)
        , m_center(std::move(b)) {}

    [[nodiscard]] auto first() const { return m_first; }
    [[nodiscard]] auto last() const { return m_last; }
    [[nodiscard]] auto size() const { return m_size; }
    [[nodiscard]] auto left() const { return m_left; }
    [[nodiscard]] auto right() const { return m_right; }
    [[nodiscard]] auto id() const { return m_id; }
    [[nodiscard]] auto centroid() const { return m_center; }

    bool operator<(const Cluster& other) const { return m_id < other.m_id; }
    bool operator==(const Cluster& other) const { return m_id == other.m_id; }

//private:
    static size_t overall_clusters_num; // number of clusters, it's used to give them unique ids
    Cluster* m_left;    //  subclusters
    Cluster* m_right;   //
    size_t m_size; // cluster size
    size_t m_first; // index of the first cluster element
    size_t m_last; // index of the last cluster element
    size_t m_id; // cluster id
    Building m_center;
};

namespace std {
template<>
struct hash<Cluster> {
    size_t operator()(const Cluster& b) const { return b.id(); }
};
}

using Clusters = std::vector<Cluster>;

using boost::numeric::ublas::matrix;

struct ClusterStructure {
    ClusterStructure() = delete;

    ClusterStructure(const Map& map, Buildings&& buildings, DMatrix<Building>&& dm);

    auto merge_clusters(size_t id1, size_t id2) -> Cluster;

    void print_cluster_structure(std::ostream& out = std::cout, const Cluster* curr_cl = nullptr,
                                 int level = 0) const;

    auto get_elements(size_t id) const -> std::vector<Building>;

    [[nodiscard]] auto root() const { return m_root; }
    [[nodiscard]] auto clusters() const { return m_clusters; }

    void cluster_from_element(size_t ind, Building b);

    auto clusters_num() const { return m_clusters_num; }

private:
    const Cluster* m_root = nullptr;
    Buildings m_data;
    Clusters m_clusters {};
    DMatrix<Building> m_dm_buildings;
    matrix<uint64_t> m_dm_clusters;
    std::vector<int64_t> _m_next;
    size_t m_clusters_num;
    const Map& m_map;
};

#endif //CLUSTERING_HPP
