struct Map {
	struct Path {
	private:
		Building m_from, m_to;
		Distance m_distance;
	};

	struct TracedPath: public Path {
	private:
		vector<Node> m_trace;
	};
	
private:
	Buildings m_buildings;
	Graph m_graph;
};
