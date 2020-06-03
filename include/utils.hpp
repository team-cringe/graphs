#ifndef GRAPHS_UTILS_HPP
#define GRAPHS_UTILS_HPP

#include <utility>
#include <vector>
#include <fstream>
#include <filesystem>
#include <numeric>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/way.hpp>

namespace graphs {
using Distance = double;
using Angle = long double;
using Location = std::pair<Angle, Angle>;
using Locations = std::vector<Location>;

template<typename T>
bool serialize(const std::string& filename, T&& data) {
    std::ofstream binary { filename, std::ios::out | std::ios::binary | std::ios::app };
    boost::archive::binary_oarchive archive { binary, boost::archive::no_header };
    archive << data;
    binary.close();
    return true;
}

template<typename T>
bool deserialize(const std::string& filename, T&& data) {
    if (!std::filesystem::exists(filename)) { return false; }
    std::ifstream binary { filename, std::ios::binary };
    boost::archive::binary_iarchive archive { binary, boost::archive::no_header };
    archive >> data;
    binary.close();
    return true;
}

/**
 * Factory method for Position.
 */
inline auto make_pos(const osmium::NodeRef& node) -> Location {
    return { node.lat(), node.lon() };
}

/**
 * Determines the great-circle distance between two points given their longitudes and latitudes.
 *
 * @param x, y OSM nodes with corresponding coordinates.
 * @return Distance between nodes.
 */
inline auto haversine(const Location& x, const Location& y) -> Distance {
    const auto[lat_1, lon_1] = x;
    const auto[lat_2, lon_2] = y;
    constexpr long double R = 6'371'000;

    // Convert to radians
    const auto phi1 = lat_1 * M_PI / 180;
    const auto phi2 = lat_2 * M_PI / 180;
    const auto d_phi = (lat_2 - lat_1) * M_PI / 180;
    const auto d_lambda = (lon_2 - lon_1) * M_PI / 180;

    // Square of half the chord length between the objects
    const auto a = std::pow(std::sin(d_phi / 2), 2) +
                   std::cos(phi1) * std::cos(phi2) * std::pow(std::sin(d_lambda / 2), 2);
    // Angular distance in radians
    const auto c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

/**
 * Determines the geographical center of a building consisting of ambient nodes.
 *
 * @param nodes List of nodes of an OSM way.
 * @return Geocenter described by a pair of latitude and longitude respectively.
 */
inline auto barycenter(const osmium::WayNodeList& nodes) -> Location {
    const auto lat = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lat(); });
    const auto lon = std::accumulate(nodes.cbegin(), nodes.cend(), static_cast<long double>(0),
                                     [](auto lhs, const auto& node) { return lhs + node.lon(); });
    const auto num = nodes.size();

    return { lat / num, lon / num };
}

inline auto barycenter(const Locations& locations) -> Location {
    const auto
        lat = std::accumulate(locations.cbegin(), locations.cend(), static_cast<long double>(0),
                              [](auto lhs, const auto& node) { return lhs + node.first; });
    const auto
        lon = std::accumulate(locations.cbegin(), locations.cend(), static_cast<long double>(0),
                              [](auto lhs, const auto& node) { return lhs + node.second; });
    const auto num = locations.size();

    return { lat / num, lon / num };
}
} // namespace graph


#endif // GRAPHS_UTILS_HPP
