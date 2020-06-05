unordered_map<uint64_t, bool> marked {};
 
void way(Way& way) noexcept {
	if (!way.has_key("highway")) { return; }
	for (auto& node: way.nodes()) {
		if (marked.contains(node)) {
			marked[node] |= true;
		} else {
			marked.insert({ node, false });
		}
	}
}
