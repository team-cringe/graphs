template<typename T>
bool deserialize(fs::path& filename, T&& data) {
	if (!fs::exists(filename)) { return false; }
	std::ifstream binary { filename };
	boost::binary_iarchive archive { binary };
	archive >> data;
	binary.close();
	return true;
}
