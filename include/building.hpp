#ifndef GRAPHS_BUILDING_HPP
#define GRAPHS_BUILDING_HPP

#include <boost/functional/hash.hpp>

#include "node.hpp"

namespace graphs {
struct Building {
    Building() = default;

    Building(std::uint64_t id, Location position, Distance weight, const Node& closest_node,
             unsigned char type)
        : m_id(id)
        , m_weight(weight)
        , m_closest_node(closest_node) {
        m_latitude = position.first;
        m_longitude = position.second;
        if (type == 0) { m_type = Type::House; }
        else { m_type = Type::Facility; }
    }

    bool operator==(const Building& other) const { return m_id == other.m_id; }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& archive, const unsigned int& version) {
        (void) version;
        archive & m_id;
        archive & m_type;
        archive & m_latitude;
        archive & m_longitude;
        archive & m_weight;
        archive & m_closest_node;
    }

    [[nodiscard]]
    [[maybe_unused]]
    bool is_house() const { return m_type == Type::House; }
    [[nodiscard]]
    [[maybe_unused]]
    bool is_facility() const { return m_type == Type::Facility; }

    [[nodiscard]] auto id() const { return m_id; }
    [[nodiscard]] const Node& closest() const { return m_closest_node; }
    [[nodiscard]] Location location() const { return { m_latitude, m_longitude }; }
    [[nodiscard]] auto longitude() const { return m_longitude; }
    [[nodiscard]] auto latitude() const { return m_latitude; }
    [[nodiscard]] auto weight() const { return m_weight; }

private:
    enum class Type {
        House,
        Facility
    };

    std::uint64_t m_id = 0;
    Type m_type = Type::House;
    Angle m_latitude = 0, m_longitude = 0;
    Distance m_weight = 0;
    Node m_closest_node {};
};

/**
 * Factory method for Building.
 * Calculates its position and resolves correct type based on OSM data.
 *
 * @details Use RTTI in order to define correct type in hierarchy.
 *
 * @param way OSM way that represents building (or you gonna catch runtime error, lol).
 */
inline auto make_building(const osmium::Way& way, Location location, const Node& closest) -> Building {
    const auto type = way.tags().get_value_by_key("building");
    const std::vector<std::string> houses =
        { "apartments", "bungalow", "cabin", "detached", "dormitory", "farm", "ger", "hotel",
          "house", "houseboat", "residential", " semidetached_house", "static_caravan", "terrace" };
    for (const auto& house: houses) {
        if (type == house) { return Building { way.positive_id(), location, 0, closest, 0 }; }
    }
    return Building { way.positive_id(), location, 0, closest, 1 };
}

using Buildings = std::vector<Building>;
} // namespace graphs

namespace std {
template<>
struct hash<graphs::Building> {
    size_t operator()(const graphs::Building& b) const {
        using boost::hash_value;
        using boost::hash_combine;

        size_t seed = 0;
        hash_combine(seed, hash_value(b.id()));
        hash_combine(seed, hash_value(b.latitude()));
        hash_combine(seed, hash_value(b.longitude()));
        hash_combine(seed, hash_value(b.weight()));
        return seed;
    }
};
} // namespace std

#endif // GRAPHS_BUILDING_HPP
