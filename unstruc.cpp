#include <iostream>
#include <string>

#include "error.h"
#include "grid.h"
#include "su2.h"
#include "gmsh.h"
#include "block.h"
#include "translation.h"
#include "plot3d.h"

#define TOL 3.e-8

int main (int argc, char* argv[])
{
	if (argc < 2) {
		Fatal("One argument required");
	}
	int i = 1;
	char * blockfile = NULL;
	char * translationfile = NULL;
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (std::string(argv[i]) == "-t") {
				i++;
				if (i == argc) Fatal("Must filename option to -t");
				translationfile  = argv[i];
			}
		} else {
			if (blockfile) Fatal("blockname defined twice");
			blockfile = argv[i];
		}
		i++;
	}
	MultiBlock * mb = ReadPlot3D(blockfile);
	Grid * grid = to_grid(mb);
	sort_points(grid);
	merge_points(grid,TOL);
	sort_elements(grid);
	delete_inner_faces(grid);
	collapse_elements(grid);
	if (translationfile) {
		TranslationTable * transt = ReadTranslationFile(translationfile,size(mb));
		applyTranslation(grid,transt);
	}
	toSU2(grid);
	return 0;
}
