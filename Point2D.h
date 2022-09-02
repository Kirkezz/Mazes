#pragma once
#include <cmath>
#include <concepts>
template<typename T>
struct Point2D {
	T x, y;
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
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Point2D& operator-=(const Point2D& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Point2D& operator/=(const Point2D& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	bool operator==(const Point2D& rhs) const { return rhs.x == x && rhs.y == y; }
	bool operator!=(const Point2D& rhs) const { return rhs.x != x || rhs.y != y; }
	operator bool() const requires std::same_as<T, float> { return std::isfinite(x) && std::isfinite(y); }
};
using Point2Du = Point2D<size_t>;
using Point2Df = Point2D<float>;
