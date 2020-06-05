struct ClusterStructure {
    const Cluster* m_root = nullptr;
    Buildings m_data;
    Clusters m_clusters {};
    DMatrix<Building> m_dm_buildings;
    matrix<uint64_t> m_dm_clusters;
    std::vector<int64_t> _m_next;
    size_t m_clusters_num;
    const Map& m_map;
};