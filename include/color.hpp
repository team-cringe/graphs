#ifndef COLOR_HPP
#define COLOR_HPP

#include <vector>
#include <string>

struct Color {
/*
 * 0 <= r <= 1
 * 0 <= g <= 1
 * 0 <= b <= 1
 * 0 <= h <= 360
 * 0 <= s <= 1
 * 0 <= v <= 1
 */
    double& r() { return components[0]; }
    double& g() { return components[1]; }
    double& b() { return components[2]; }

    double& h() { return components[0]; }
    double& s() { return components[1]; }
    double& v() { return components[2]; }

    static const Color gray;

    [[nodiscard]] std::string hex();
    double components[3] { 0, 0, 0 };
};

using Colors = std::vector<Color>;

/**
 * Convert hsv to rgb
 * Source: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
 * @param hsv
 * @return
 */
auto hsv_to_rgb(Color hsv) -> Color;

/**
 * Generate n random colors in rgb format
 * Source: https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
 * @param n
 * @return vector of colors
 */
auto generate_colors(size_t n, double s = 0.5, double v = 0.95) -> Colors;

#endif //COLOR_HPP
