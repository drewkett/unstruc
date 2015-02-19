#ifndef POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10
#define POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10

#include <cmath>

struct Vector
{
	double x,y,z;

	Vector() : x(0), y(0), z(0) {};
	Vector(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};

	inline Vector operator/(double value) {return Vector (x/value, y/value, z/value);};
	inline Vector operator*(double value) {return Vector (x*value, y*value, z*value);};
	inline void operator+=(Vector other) {x += other.x; y += other.y; z += other.z;};
	inline void operator-=(Vector other) {x -= other.x; y -= other.y; z -= other.z;};
	inline void operator/=(double value) {x /= value; y /= value; z /= value;};
	inline double dot(Vector& other) { return x*other.x + y*other.y + z*other.z; };
	inline double length() {return sqrt(x*x + y*y + z*z);};
};

struct Point
{
	double x,y,z;

	Point () : x(0), y(0), z(0) {};
	Point (double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};
	inline Vector operator-(Point other) { return Vector (x-other.x,y-other.y,z-other.z); }

	inline Point operator-(Vector other) { return Point (x-other.x,y-other.y,z-other.z); }
	inline Point operator+(Vector other) { return Point (x+other.x,y+other.y,z+other.z); }

	inline void operator-=(Vector other) {x -= other.x; y -= other.y; z -= other.z;};
	inline void operator+=(Vector other) {x += other.x; y += other.y; z += other.z;};
};

void dump(Point& p);
bool same(Point& p1, Point& p2, double tol);

void dump(Vector&);
double angle_between(const Vector&, const Vector&);
double dot(const Vector&, const Vector&);
Vector cross(const Vector&, const Vector&);

#endif
