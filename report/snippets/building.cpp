struct Building {
private:
	enum class Type {
		House,
		Facility
	};

	uint64_t m_id = 0;
	Type m_type;
	Angle m_latitude = 0;
	Angle m_longitude = 0;
	Node m_closest_node; // Closest road node.
};
