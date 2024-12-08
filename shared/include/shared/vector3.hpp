#ifndef VECTOR3_HPP_
#define VECTOR3_HPP_

#include <cmath>
#include <iostream>

struct Vector3 {
  float x;
  float y;
  float z;

  Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
  Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

  Vector3 operator+(const Vector3& other) const {
    return {x + other.x, y + other.y, z + other.z};
  }

  Vector3 operator-(const Vector3& other) const {
    return {x - other.x, y - other.y, z - other.z};
  }

  Vector3 operator*(float scalar) const {
    return {x * scalar, y * scalar, z * scalar};
  }

  Vector3 operator/(float scalar) const {
    return {x / scalar, y / scalar, z / scalar};
  }

  float length() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  Vector3 normalized() const {
    float len = length();
    return len > 0.0f ? (*this) / len : Vector3();
  }

  bool operator==(const Vector3& other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  bool operator!=(const Vector3& other) const {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
  }
};

#endif // VECTOR3_HPP_
