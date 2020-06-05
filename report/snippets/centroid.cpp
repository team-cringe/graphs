Location loc;
loc.first = (
    cl1.centroid().location().first * cl1.size() +
    cl2.centroid().location().first * cl2.size())
    / (cl1.size() + cl2.size());
loc.second = (
    cl1.centroid().location().second * cl1.size() +
    cl2.centroid().location().second * cl2.size())
    / (cl1.size() + cl2.size());

auto b = find_nearest_building(m_map, loc);