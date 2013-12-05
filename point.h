struct Point
{
	double x,y,z,s,i;
};

void dump(Point * p);
void dump(Point ** p);
void set_s(Point * p);
bool comparePoint(Point * p1, Point * p2);
bool comparePPoint(Point ** p1, Point ** p2);
bool close(Point * p1, Point * p2, double tol);
bool same(Point * p1, Point * p2, double tol);
