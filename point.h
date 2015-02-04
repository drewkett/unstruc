#ifndef POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10
#define POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10

#include <cmath>

struct Vector
{
	double x,y,z;
	Vector() : x(0), y(0), z(0) {};
	Vector(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};

	Vector operator/(double value) {return Vector (x/value, y/value, z/value);};
	void operator+=(Vector other) {x += other.x; y += other.y; z += other.z;};
	double dot(Vector other) { return x*other.x + y*other.y + z*other.z; };
	double length() {return sqrt(x*x + y*y + z*z);};
};

struct Point
{
	double x,y,z,s;
	int i;
	Point () : x(0), y(0), z(0), s(0), i(0) {};
	Point (double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};
	Vector operator-(Point other) { return Vector (x-other.x,y-other.y,z-other.z); }
};

void dump(Point * p);
void dump(Point ** p);
bool compare_point(Point * p1, Point * p2);
bool compare_point_by_index(Point * p1, Point * p2);
bool compare_ppoint(Point ** p1, Point ** p2);
bool compare_ppoint_by_index(Point ** p1, Point ** p2);
bool close(Point &p1, Point &p2, double tol);
bool same(Point &p1, Point &p2, double tol);

void dump(Vector&);
Vector subtract_points(Point *, Point *);
double angle_between(const Vector&, const Vector&);
double dot(const Vector&, const Vector&);
Vector cross(const Vector&, const Vector&);

#endif
