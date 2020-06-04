#include "graph.hpp"
#include "clustering.hpp"

size_t Cluster::overall_clusters_num = 0;

ClusterStructure::ClusterStructure(Buildings&& buildings, DMatrix<Building>&& dm)
    : m_data(buildings)
    , m_dm_buildings(dm)
    , m_dm_clusters(2 * buildings.size() - 1, 2 * buildings.size() - 1)
    , m_clusters_num(0) {

//        for every building create cluster
    m_clusters.reserve(2 * m_data.size() - 1);
    _m_next.resize(2 * m_data.size() - 1);
    for (size_t i = 0; i < m_data.size(); ++i) {
        cluster_from_element(i, { m_data[i].longitude(), m_data[i].latitude() });
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
    _m_next[m_clusters[id1].last()] = m_clusters[id2].first();
    ++m_clusters_num;
    return Cluster(m_clusters[id1], m_clusters[id2]);
}

void ClusterStructure::cluster_from_element(size_t ind, Location loc) {
    m_clusters.emplace_back(ind, loc);
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