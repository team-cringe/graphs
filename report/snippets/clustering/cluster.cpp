struct Cluster {
    static size_t overall_clusters_num;
    Cluster* m_left;    //  subclusters
    Cluster* m_right;   //
    size_t m_size; // cluster size
    size_t m_first; // index of the first element
    size_t m_last; // index of the last element
    size_t m_id; // cluster id
    Building m_center;
};