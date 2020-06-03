#include "assessment.hpp"

#include <iostream>

void assessment(const graphs::Map& map, int houses_num, int facilities_num) {
    auto houses = map.select_random_houses(houses_num);
    auto facilities = map.select_random_facilities(facilities_num);
    for (const auto& house: houses) {
        auto paths = map.shortest_paths(house, facilities);
        auto closest = std::min_element(paths.cbegin(), paths.cend(),
                                        [](const auto& a, const auto& b) {
                                            return a.distance() < b.distance();
                                        });
        auto[from, to] = closest->ends();
        std::cout << "Closest facility from " << from.latitude() << "/" << from.longitude() <<
                  " is " << to.latitude() << "/" << to.longitude() << std::endl;
    }
}
