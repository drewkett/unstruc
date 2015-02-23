#ifndef VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B
#define VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B

struct Grid;
struct Point;
struct tetgenio;

extern Point NullPoint;

Grid grid_from_tetgenio(const tetgenio& tg);
Grid volgrid_from_surface(const Grid& surface,Point& hole,double min_ratio);
Grid volgrid_from_surface(const Grid& surface,Point& hole = NullPoint,double min_ratio = 0);
Point find_point_inside_surface(const Grid& surface);

#endif
