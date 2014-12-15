#include <string>

struct Grid;

bool toSU2(std::string &outputfile, Grid * grid);
Grid * readSU2(std::string &inputfile);
