#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <algorithm>

#include "error.h"
#include "point.h"
#include "element.h"
#include "grid.h"
#include "su2.h"
#include "gmsh.h"
#include "block.h"
#include "translation.h"
#include "plot3d.h"
using namespace std;

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
			if (string(argv[i]) == "-t") {
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
	Grid * grid = toGrid(mb);
	Element * e;
	cerr << grid->points.size() << " Points" << endl;
	cerr << grid->elements.size() << " Elements" << endl;
	set_s_points(grid);
	sort(grid->ppoints.begin(),grid->ppoints.end(),comparePPoint);
	merge_points(grid,TOL);
	set_s_elements(grid);
	sort(grid->elements.begin(),grid->elements.end(),compareElement);
	delete_inner_faces(grid);
	collapse_elements(grid);
	if (translationfile) {
		TranslationTable * transt = ReadTranslationFile(translationfile,size(mb));
		applyTranslation(grid,transt);
	}
	toSU2(grid);
	return 0;
}
