ClusterStructure(const Map& map, 
    Buildings&& buildings, DMatrix<Building>&& dm)
    : m_data(buildings)
    , m_dm_buildings(dm)
    , m_dm_clusters(2 * buildings.size() - 1, 
        2 * buildings.size() - 1)
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
                m_dm_buildings.at({ 
                    m_data[c1.first()],
                    m_data[c2.first()] 
                });
        }
    }