#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

class Vector
{
public:
  Vector(float nx, float ny)
    : x(nx), y(ny)
  { }
  Vector(const Vector& other)
    : x(other.x), y(other.y)
  { }
  Vector()
    : x(0), y(0)
  { }

  bool operator ==(const Vector& other) const
  {
    return x == other.x && y == other.y;
  }

  const Vector& operator=(const Vector& other)
  {
    x = other.x;
    y = other.y;
    return *this;
  }

  Vector operator+(const Vector& other) const
  {
    return Vector(x + other.x, y + other.y);
  }

  Vector operator-(const Vector& other) const
  {
    return Vector(x - other.x, y - other.y);
  }

  Vector operator*(float s) const
  {
    return Vector(x * s, y * s);
  }

  Vector operator/(float s) const
  {
    return Vector(x / s, y / s);
  }

  Vector operator-() const
  {
    return Vector(-x, -y);
  }

  const Vector& operator +=(const Vector& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  // scalar product of 2 vectors
  float operator*(const Vector& other) const
  {
    return x*other.x + y*other.y;
  }

  float norm() const;
  Vector unit() const;

  // ... add the other operators as needed, I'm too lazy now ...

  float x, y; // leave this public, get/set methods just give me headaches
              // for such simple stuff :)
};

#endif

