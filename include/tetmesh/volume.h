#ifndef VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B
#define VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B

struct Grid;
struct tetgenio;

Grid grid_from_tetgenio(tetgenio const& tg);
Grid volgrid_from_surface(Grid const& surface);

#endif
