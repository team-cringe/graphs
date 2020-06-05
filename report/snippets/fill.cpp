for (auto& curr: way.nodes()) {
	if (curr != first && curr != last) {
     	if (marked.at(curr)) {
     		auto d = ... // One-way or two-way.
			auto w = haversine(pred, curr);
			routes.add_edge({ pred, curr }, w, d);
			pred = &curr;
		}
	}
}

auto w = haversine(pred, last), d = ...;
routes.add_edge({ pred, last }, w, d);
