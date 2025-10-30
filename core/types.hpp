#pragma once
#include <cmath>

struct Vec2 {
    double x{0}, y{0};
    Vec2() = default;
    Vec2(double xx, double yy) : x(xx), y(yy) {}
    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2& operator+=(const Vec2& o){ x+=o.x; y+=o.y; return *this; }
    Vec2& operator-=(const Vec2& o){ x-=o.x; y-=o.y; return *this; }
};

inline double pow2(double e){ return std::pow(2.0, e); }
