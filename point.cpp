#include "point.h"

#include <iostream>
#include <cmath>

void dump(Point * p) {
	std::cerr << p << " -> Point " << p->x << " " << p->y << " " << p->z << std::endl;
};

void dump(Point ** p) {
	std::cerr << p << " -> ";
	dump(*p);
};

bool compare_point_by_index(Point * p1, Point * p2) {
	return p1->i < p2->i;
};

bool compare_point(Point * p1, Point * p2) {
	return p1->s < p2->s;
};

bool compare_ppoint_by_index(Point ** p1, Point ** p2) {
	if (!p1) return false;
	if (!p2) return true;
	return compare_point_by_index(*p1,*p2);
};

bool compare_ppoint(Point ** p1, Point ** p2) {
	if (!p1) return false;
	if (!p2) return true;
	return compare_point(*p1,*p2);
};

bool close(Point &p1, Point &p2, double tol) {
	return fabs(p1.s - p2.s) < 3*tol;
};

bool same(Point &p1, Point &p2, double tol) {
	return (fabs(p1.x - p2.x) < tol) && (fabs(p1.y - p2.y) < tol) && (fabs(p1.z - p2.z) < tol);
};

void dump(Vector &v) {
	printf("Vector [ %g %g %g ]\n",v.x,v.y,v.z);
};

Vector subtract_points(Point * p1, Point * p2) {
	return Vector { p1->x - p2->x, p1->y - p2->y, p1->z - p2->z };
};

double angle_between(const Vector& v1, const Vector& v2) {
	double dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	double len1 = sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z) + 1e-15;
	double len2 = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z) + 1e-15;
	return 180/M_PI*acos(dot/(len1*len2));
};
