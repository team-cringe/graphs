auto[x, y] = min;
auto new_cluster = merge_clusters(x, y);
for (auto& c: active_clusters) {
    m_dm_clusters(new_cluster.id(), c.id()) =
        std::min(m_dm_clusters(x, c.id()), 
                 m_dm_clusters(y, c.id()));
    m_dm_clusters(c.id(), new_cluster.id()) =
        std::min(m_dm_clusters(c.id(), x),
                 m_dm_clusters(c.id(), y));
}
m_dm_clusters(new_cluster.id(), new_cluster.id()) = 0;