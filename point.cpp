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
