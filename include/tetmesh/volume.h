#ifndef VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B
#define VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B

struct Grid;
struct tetgenio;

Grid grid_from_tetgenio(const tetgenio& tg);
Grid tetrahedralize_surface(const Grid& surface, double max_area);

#endif
