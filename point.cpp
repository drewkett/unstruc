#include "point.h"
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <fstream>

void dump(Point * p) {
	std::cerr << p << " -> Point " << p->x << " " << p->y << " " << p->z << std::endl;
};

void dump(Point ** p) {
	std::cerr << p << " -> ";
	dump(*p);
};

void set_s(Point * p) {
	p->s = p->x + p->y + p->z;
}

bool comparePoint(Point * p1, Point * p2) {
	return p1->s < p2->s;
};

bool comparePPoint(Point ** p1, Point ** p2) {
	return comparePoint(*p1,*p2);
};

bool close(Point * p1, Point * p2, double tol) {
	return abs(p1->s - p2->s) < 3*tol;
};

bool same(Point * p1, Point * p2, double tol) {
	return (abs(p1->x - p2->x) < tol) && (abs(p1->y - p2->y) < tol) && (abs(p1->z - p2->z) < tol);
};
