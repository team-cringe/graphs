#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include <unordered_set>
#include <boost/numeric/ublas/matrix.hpp>
#include "distance_matrix.hpp"
#include "geojson.hpp"

using namespace graph;

struct Cluster {
    Cluster() = delete;

    explicit Cluster(size_t first, Location loc)
        : m_size(1)
        , m_first(first)
        , m_last(first)
        , m_id(clusters_num++)
        , m_loc(move(loc)) {}

    explicit Cluster(Cluster& a, Cluster& b)
        : m_left(&a)
        , m_right(&b)
        , m_size(a.m_size + b.m_size)
        , m_first(a.m_first)
        , m_last(b.m_last)
        , m_id(clusters_num++)
        , m_loc(barycenter(Locations { a.m_loc, b.m_loc })) {}

    [[nodiscard]] auto first() const { return m_first; }
    [[nodiscard]] auto last() const { return m_last; }
    [[nodiscard]] auto size() const { return m_size; }
    [[nodiscard]] auto left() const { return m_left; }
    [[nodiscard]] auto right() const { return m_right; }
    [[nodiscard]] auto id() const { return m_id; }
    [[nodiscard]] auto location() const { return m_loc; }

    bool operator<(const Cluster& other) const { return m_id < other.m_id; }
    bool operator==(const Cluster& other) const { return m_id == other.m_id; }

private:
    static size_t clusters_num; // number of clusters
    Cluster* m_left = nullptr;    //  subclusters
    Cluster* m_right = nullptr;   //
    size_t m_size; // cluster size
    size_t m_first; // index of the first cluster element
    size_t m_last; // index of the last cluster element
    size_t m_id; // cluster id
    Location m_loc;
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

    ClusterStructure(Buildings&& buildings, DMatrix<Building>&& dm);

    auto merge_clusters(size_t id1, size_t id2) -> Cluster;

    void print_cluster_structure(std::ostream& out = std::cout, const Cluster* curr_cl = nullptr,
                                 int level = 0) const;

    auto get_elements(size_t id) const -> std::vector<Building>;

    [[nodiscard]] auto root() const { return m_root; }
    [[nodiscard]] auto clusters() const { return m_clusters; }

    auto to_geojson() const -> json;

//private:
    const Cluster* m_root = nullptr;
    Buildings m_data;
    Clusters m_clusters {};
    DMatrix<Building> m_dm_buildings;
    matrix<uint64_t> m_dm_clusters;
    std::vector<int64_t> _m_next;
};

#endif //CLUSTERING_HPP
