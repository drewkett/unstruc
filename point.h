#ifndef POINT_H_18F17439_7F9C_4610_91ED_D673F59EBB10
#define POINT_H_18F17439_7F9C_4610_91ED_D673F59EBB10

struct Point
{
	double x,y,z,s;
	int i;
};

void dump(Point * p);
void dump(Point ** p);
bool compare_point(Point * p1, Point * p2);
bool compare_point_by_index(Point * p1, Point * p2);
bool compare_ppoint(Point ** p1, Point ** p2);
bool compare_ppoint_by_index(Point ** p1, Point ** p2);
bool close(Point &p1, Point &p2, double tol);
bool same(Point &p1, Point &p2, double tol);

#endif
