struct Point
{
	double x,y,z,s,i;
};

void dump(Point * p);
void dump(Point ** p);
bool compare_point(Point * p1, Point * p2);
bool compare_ppoint(Point ** p1, Point ** p2);
bool close(Point * p1, Point * p2, double tol);
bool same(Point * p1, Point * p2, double tol);
