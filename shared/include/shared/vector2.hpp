#ifndef VECTOR2_HPP_
#define VECTOR2_HPP_

#include <cmath>
#include <iostream>

struct Vector2 {
  float x;
  float y;

  Vector2() : x(0.0f), y(0.0f) {}
  Vector2(float x, float y) : x(x), y(y) {}

  Vector2 operator+(const Vector2& other) const {
    return {x + other.x, y + other.y};
  }

  Vector2 operator-(const Vector2& other) const {
    return {x - other.x, y - other.y};
  }

  Vector2 operator*(float scalar) const {
    return {x * scalar, y * scalar};
  }

  Vector2 operator/(float scalar) const {
    return {x / scalar, y / scalar};
  }

  float length() const {
    return std::sqrt(x * x + y * y);
  }

  Vector2 normalized() const {
    float len = length();
    return len > 0.0f ? (*this) / len : Vector2();
  }

  bool operator==(const Vector2& other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Vector2& other) const {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }
};

#endif // VECTOR2_HPP_
