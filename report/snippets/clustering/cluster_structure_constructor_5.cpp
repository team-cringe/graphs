m_clusters.emplace_back(new_cluster);
ac.insert(new_cluster);
ac.erase(m_clusters[x]);
ac.erase(m_clusters[y]);