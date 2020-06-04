#ifndef GRAPHS_NODE_HPP
#define GRAPHS_NODE_HPP

#include <boost/functional/hash.hpp>
#include <boost/serialization/access.hpp>

#include "utils.hpp"

namespace graphs {
struct Node {
    explicit Node() = default;

    explicit Node(std::uint64_t id)
        : m_id(id) {};

    Node(std::uint64_t id, Angle latitude, Angle longitude)
        : m_id(id)
        , m_latitude(latitude)
        , m_longitude(longitude) {};

    Node(std::uint64_t id, Location location)
        : m_id(id)
        , m_latitude(location.first)
        , m_longitude(location.second) {};

    [[nodiscard]] std::uint64_t id() const { return m_id; }
    [[nodiscard]] Angle latitude() const { return m_latitude; }
    [[nodiscard]] Angle longitude() const { return m_longitude; }
    [[nodiscard]] auto location() const -> Location { return { m_latitude, m_longitude }; }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& archive, const unsigned int& version) {
        (void) version;
        archive & m_id;
        archive & m_latitude;
        archive & m_longitude;
    }

    bool operator==(const Node& other) const { return other.m_id == m_id; }
    bool operator!=(const Node& other) const { return !(other == *this); }
    bool operator<(const Node& other) const { return m_id < other.m_id; }

private:
    std::uint64_t m_id = 0;
    Angle m_latitude = 0;
    Angle m_longitude = 0;
};

using Nodes = std::vector<Node>;

/**
 * Factory function for Node.
 */
inline auto make_node(const osmium::NodeRef& node) -> Node {
    return { node.positive_ref(), node.lat(), node.lon() };
}
} // namespace graphs

namespace std {
template<>
struct hash<graphs::Node> {
    size_t operator()(const graphs::Node& n) const {
        using boost::hash_value;
        using boost::hash_combine;

        size_t seed = 0;
        hash_combine(seed, hash_value(n.id()));
        hash_combine(seed, hash_value(n.longitude()));
        hash_combine(seed, hash_value(n.latitude()));
        return seed;
    }
};
} // namespace std

#endif // GRAPHS_NODE_HPP
