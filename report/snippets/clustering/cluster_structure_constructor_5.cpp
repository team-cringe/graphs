m_clusters.emplace_back(new_cluster);
active_clusters.insert(new_cluster);
active_clusters.erase(m_clusters[x]);
active_clusters.erase(m_clusters[y]);