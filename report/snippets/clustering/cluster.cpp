struct Cluster {
    static size_t overall_clusters_num;
    Cluster* m_left;   // Subclusters.
    Cluster* m_right;  
    size_t m_size;     // Cluster size.
    size_t m_first;    // Index of the first element.
    size_t m_last;     // Index of the last element.
    size_t m_id;       // Cluster ID.
    Building m_center;
};
