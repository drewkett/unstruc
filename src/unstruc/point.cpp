#include "point.h"

#include <iostream>
#include <cmath>

namespace unstruc {

void dump(Point p) {
	printf("Point %8.6g %8.6g %8.6g\n",p.x,p.y,p.z);
};

bool same(Point& p1, Point& p2, double tol) {
	return (fabs(p1.x - p2.x) <= tol) && (fabs(p1.y - p2.y) <= tol) && (fabs(p1.z - p2.z) <= tol);
};

void dump(Vector v) {
	printf("Vector [ % 7.2e % 7.2e % 7.2e ]\n",v.x,v.y,v.z);
};

double angle_between(const Vector& v1, const Vector& v2) {
	double dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	double len1 = sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z) + 1e-15;
	double len2 = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z) + 1e-15;
	return 180/M_PI*acos(dot/(len1*len2));
};

double dot(const Vector& v1, const Vector& v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
};

Vector cross(const Vector& v1, const Vector& v2) {
	Vector v;
	v.x = v1.y*v2.z - v1.z*v2.y;
	v.y = v1.z*v2.x - v1.x*v2.z;
	v.z = v1.x*v2.y - v1.y*v2.x;
	return v;
};

} // namespace unstruc
