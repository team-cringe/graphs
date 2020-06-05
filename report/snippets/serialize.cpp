template<typename T>
bool serialize(fs::path& filename, T&& data) {
	std::ofstream binary { filename };
	boost::binary_oarchive archive { binary };
	archive << data;
	binary.close();
	return true;
}
