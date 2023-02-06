#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <concepts>
template <typename T> struct Point2D {
    T x, y;
    explicit Point2D(sf::Vector2f p)
        requires std::same_as<T, float>
        : x(p.x), y(p.y) {}
    Point2D() : x(), y() {}
    Point2D(T x, T y) : x(x), y(y) {}
    Point2D(T v) : x(v), y(v) {}
    Point2D operator+(Point2D rhs) { return Point2D(x + rhs.x, y + rhs.y); }
    Point2D operator*(Point2D rhs) { return Point2D(x * rhs.x, y * rhs.y); }
    Point2D operator-(Point2D rhs) { return Point2D(x - rhs.x, y - rhs.y); }
    Point2D operator/(Point2D rhs) { return Point2D(x / rhs.x, y / rhs.y); }
    Point2D& operator+=(const Point2D& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Point2D& operator*=(const Point2D& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }
    Point2D& operator-=(const Point2D& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Point2D& operator/=(const Point2D& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }
    bool operator==(const Point2D& rhs) const { return rhs.x == x && rhs.y == y; }
    bool operator!=(const Point2D& rhs) const { return rhs.x != x || rhs.y != y; }
    operator bool() const
        requires std::same_as<T, float>
    {
        return std::isfinite(x) && std::isfinite(y);
    }
    operator sf::Vector2f() const
        requires std::same_as<T, float>
    {
        return sf::Vector2f(x, y);
    }
    float distance(const Point2D<T>& other) const { return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2)); }
    bool operator<(const Point2D<T>& other) const { return (x != other.x) ? (x > other.x) : (other.y < y); }
};
using Point2Du = Point2D<size_t>;
using Point2Df = Point2D<float>;
