#ifndef PLOT3D_H_FB2FB01B_ABBD_4A5A_B5BE_9C58E6293D9A
#define PLOT3D_H_FB2FB01B_ABBD_4A5A_B5BE_9C58E6293D9A

#include <string> 

struct MultiBlock;
struct Grid;

void readPlot3D(MultiBlock&, std::string &filename);
void readPlot3DToGrid(Grid&, std::string &filename);

#endif
