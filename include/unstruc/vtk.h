#ifndef VTK_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E
#define VTK_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E

#include <string>

struct Grid;

bool toVTK(std::string outputfile, Grid &grid);

#endif