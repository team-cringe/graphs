struct Graph {
private:
	using Edge = pair<Node, Node>;
	using OutEdges = unordered_map<Node, Distance>;
	using AdjList = unordered_map<Node, OutEdges>;

	AdjList m_data {};
};
