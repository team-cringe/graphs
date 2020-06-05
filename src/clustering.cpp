#include "graph.hpp"
#include "clustering.hpp"

auto centroid(const Map& map, Locations locs) -> Building {
    return find_nearest_building(map, barycenter(locs));
}

auto find_nearest_building(const Map& map, Location loc) -> Building {
    return *std::min_element(map.buildings().begin(), map.buildings().end(),
                             [&](const auto& lhs, const auto& rhs) {
                                 return
                                     haversine(lhs.location(), loc) <
                                     haversine(rhs.location(), loc);
                             });
}

size_t Cluster::overall_clusters_num = 0;

ClusterStructure::ClusterStructure(const Map& map, Buildings&& buildings, DMatrix<Building>&& dm)
    : m_data(buildings)
    , m_dm_buildings(dm)
    , m_dm_clusters(2 * buildings.size() - 1, 2 * buildings.size() - 1)
    , m_clusters_num(0)
    , m_map(map) {

//        for every building create cluster
    m_clusters.reserve(2 * m_data.size() - 1);
    _m_next.resize(2 * m_data.size() - 1);
    for (size_t i = 0; i < m_data.size(); ++i) {
        cluster_from_element(i, m_data[i]);
    }

//        create distance matrix for clusters
    for (auto& c1: m_clusters) {
        for (auto& c2: m_clusters) {
            m_dm_clusters(c1.id(), c2.id()) =
                m_dm_buildings.at({ m_data[c1.first()], m_data[c2.first()] });
        }
    }

//        stores all clusters that aren't included in other clusters
    std::unordered_set<Cluster> active_clusters(m_clusters.begin(), m_clusters.end());

    while (active_clusters.size() > 1) {

//            find minimal distance among active clusters
        std::pair<size_t, size_t> min = { -1, -1 };
        for (auto& c1: active_clusters) {
            for (auto& c2: active_clusters) {
                if (c1.id() == c2.id()) { continue; }

                if (min.first > m_clusters.size()) {
                    min = { c1.id(), c2.id() };
                    continue;
                }

                if (m_dm_clusters(c1.id(), c2.id()) < m_dm_clusters(min.first, min.second)) {
                    min = { c1.id(), c2.id() };
                }
            }
        }

        auto[x, y] = min;
        auto new_cluster = merge_clusters(x, y);

//            calculate distances for new cluster
        for (auto& c: active_clusters) {
            m_dm_clusters(new_cluster.id(), c.id()) =
                std::min(m_dm_clusters(x, c.id()), m_dm_clusters(y, c.id()));
            m_dm_clusters(c.id(), new_cluster.id()) =
                std::min(m_dm_clusters(c.id(), x), m_dm_clusters(c.id(), y));
        }
        m_dm_clusters(new_cluster.id(), new_cluster.id()) = 0;

//            add new cluster
        m_clusters.emplace_back(new_cluster);
        active_clusters.insert(new_cluster);

//            delete merged clusters from active clusters
        active_clusters.erase(m_clusters[x]);
        active_clusters.erase(m_clusters[y]);
    }

//        last added cluster is root
    m_root = &(*m_clusters.rbegin());
}

auto ClusterStructure::merge_clusters(size_t id1, size_t id2) -> Cluster {
    auto cl1 = m_clusters[id1];
    auto cl2 = m_clusters[id2];
    _m_next[cl1.last()] = cl2.first();

    Location loc;
    loc.first = (cl1.centroid().location().first * cl1.size() +
                 cl2.centroid().location().first * cl2.size()) / (cl1.size() + cl2.size());
    loc.second = (cl1.centroid().location().second * cl1.size() +
                  cl2.centroid().location().second * cl2.size()) / (cl1.size() + cl2.size());

    auto b = find_nearest_building(m_map, loc);

    ++m_clusters_num;
    return Cluster(m_clusters[id1], m_clusters[id2], b);
}

void ClusterStructure::cluster_from_element(size_t ind, Building b) {
    m_clusters.emplace_back(ind, b);
    _m_next[ind] = -1;
    ++m_clusters_num;
}

void ClusterStructure::print_cluster_structure(std::ostream& out, const Cluster* curr_cl,
                                               int level) const {
    curr_cl = curr_cl == nullptr ? m_root : curr_cl;
    out << std::string(level, '-') + ' ' << curr_cl->id() << ':' << curr_cl->size()
        << std::endl;
    if (curr_cl->left()) {
        print_cluster_structure(out, curr_cl->left(), level + 2);
    }
    if (curr_cl->right()) {
        print_cluster_structure(out, curr_cl->right(), level + 2);
    }
}

auto ClusterStructure::get_elements(size_t id) const -> std::vector<Building> {
    auto cl = m_clusters[id];
    size_t i = cl.first();
    std::vector<Building> elements;
    elements.reserve(cl.size());
    while (i != cl.last()) {
        elements.emplace_back(m_data[i]);
        i = _m_next[i];
    }
    elements.emplace_back(m_data[i]);
    return elements;
}

Clusters get_k_clusters(const ClusterStructure& cl_st, size_t k) {
    if (k > cl_st.clusters().size()) {
        return Clusters {};
    }
    struct comp {
        bool operator()(const Cluster& lhs, const Cluster& rhs) const {
            return lhs.size() < rhs.size()
                   ? false
                   : lhs.size() > rhs.size()
                     ? true
                     : lhs.id() < rhs.id();
        };
    };
    std::set<Cluster, comp> clusters;
    clusters.insert(*cl_st.root());

    while (clusters.size() < k) {
        auto cl = *clusters.begin();
        clusters.insert(*(cl.left()));
        clusters.insert(*(cl.right()));
        clusters.erase(cl);
    }

    return Clusters(clusters.begin(), clusters.end());
}

Maps clusters_to_maps(const Map& map, const Clusters& cls, const ClusterStructure& cl_st) {
    Maps maps;
    maps.reserve(cls.size());
    for (auto& cl: cls) {
        auto buildings = cl_st.get_elements(cl.id());
        auto paths = map.shortest_paths_with_trace(cl.centroid(), buildings);
        maps.emplace_back(paths_to_map(map, paths));
    }
    return maps;
}