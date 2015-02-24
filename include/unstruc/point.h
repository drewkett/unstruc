#ifndef POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10
#define POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10

#include <cmath>

struct Vector
{
	double x,y,z;

	Vector() : x(0), y(0), z(0) {};
	Vector(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};

	inline Vector operator*(double value) const {return Vector(*this) *= value;};
	inline Vector operator/(double value) const {return Vector(*this) /= value;};
	inline Vector& operator*=(double value) {x *= value; y *= value; z *= value; return *this;};
	inline Vector& operator/=(double value) {x /= value; y /= value; z /= value; return *this;};

	inline Vector operator+(const Vector& other) const {return Vector(*this) += other;};
	inline Vector operator-(const Vector& other) const {return Vector(*this) -= other;};
	inline Vector& operator+=(const Vector& other) {x += other.x; y += other.y; z += other.z; return *this;};
	inline Vector& operator-=(const Vector& other) {x -= other.x; y -= other.y; z -= other.z; return *this;};

	inline double dot(const Vector& other) const { return x*other.x + y*other.y + z*other.z; };
	inline double length() const {return sqrt(x*x + y*y + z*z);};
};
template<typename T>
Vector operator*(T const& value, Vector vec) {
	return vec *= value;
}

struct Point
{
	double x,y,z;

	Point () : x(0), y(0), z(0) {};
	Point (double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};
	inline bool operator==(Point other) {return x == other.x && y == other.y && z == other.z;}

	inline Point operator/(double value) { return Point (x/value,y/value,z/value); }

	//inline Point operator+(Point other) { return Point (x+other.x,y+other.y,z+other.z); }
	inline Vector operator-(const Point& other) const { return Vector (x-other.x,y-other.y,z-other.z);}

	inline Point operator-(const Vector& other) const { return Point (x-other.x,y-other.y,z-other.z); }
	inline Point operator+(const Vector& other) const { return Point (x+other.x,y+other.y,z+other.z); }

	inline void operator-=(const Vector& other) {x -= other.x; y -= other.y; z -= other.z;};
	inline void operator+=(const Vector& other) {x += other.x; y += other.y; z += other.z;};
};

void dump(Point p);
bool same(Point& p1, Point& p2, double tol);

void dump(Vector);
double angle_between(const Vector&, const Vector&);
double dot(const Vector&, const Vector&);
Vector cross(const Vector&, const Vector&);

#endif
