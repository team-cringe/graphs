#include "color.hpp"

#include <cmath>
#include <random>
#include <limits>
#include <ctime>
#include <sstream>
#include <iomanip>

const Color Color::gray = { 0.33, 0.33, 0.33 };

std::string Color::hex() {
    if (r() < 0 or g() < 0 or b() < 0 or r() > 1 or g() > 1 or b() > 1) {
        return std::string { "000000" };
    }

    int red = static_cast<int>(r() * 255);
    int green = static_cast<int>(g() * 255);
    int blue = static_cast<int>(b() * 255);
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(2) << red << std::setw(2) << green
           << std::setw(2) << blue;
    return stream.str();
}

auto hsv_to_rgb(Color hsv) -> Color {
    Color rgb;

    double fC = hsv.v() * hsv.s(); // Chroma
    double fHPrime = fmod(hsv.h() / 60.0, 6);
    double fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    double fM = hsv.v() - fC;

    if (0 <= fHPrime && fHPrime < 1) {
        rgb.r() = fC;
        rgb.g() = fX;
        rgb.b() = 0;
    } else if (1 <= fHPrime && fHPrime < 2) {
        rgb.r() = fX;
        rgb.g() = fC;
        rgb.b() = 0;
    } else if (2 <= fHPrime && fHPrime < 3) {
        rgb.r() = 0;
        rgb.g() = fC;
        rgb.b() = fX;
    } else if (3 <= fHPrime && fHPrime < 4) {
        rgb.r() = 0;
        rgb.g() = fX;
        rgb.b() = fC;
    } else if (4 <= fHPrime && fHPrime < 5) {
        rgb.r() = fX;
        rgb.g() = 0;
        rgb.b() = fC;
    } else if (5 <= fHPrime && fHPrime < 6) {
        rgb.r() = fC;
        rgb.g() = 0;
        rgb.b() = fX;
    } else {
        rgb.r() = 0;
        rgb.g() = 0;
        rgb.b() = 0;
    }

    rgb.r() += fM;
    rgb.g() += fM;
    rgb.b() += fM;

    return rgb;
}

const double golden_ratio_conjugate = 0.618033988749895;
auto generate_colors(size_t n, double s, double v) -> Colors {
    std::default_random_engine gen((std::random_device()()));
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
    double h = fmod(dist(gen), 360);

    Colors colors;
    colors.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        colors.emplace_back(hsv_to_rgb(Color { h, s, v }));
        h = fmod(h + golden_ratio_conjugate * 360., 360);
    }

    return colors;
}