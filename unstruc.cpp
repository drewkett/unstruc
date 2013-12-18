#include <iostream>
#include <string>

#include "error.h"
#include "grid.h"
#include "su2.h"
#include "vtk.h"
#include "gmsh.h"
#include "block.h"
#include "translation.h"
#include "plot3d.h"

#define TOL 3.e-8

enum BLOCKTYPE {
	UNKNOWN = 0,
	PLOT3D = 1,
	SU2 = 2,
	VTK = 3
};

int get_blocktype ( std::string * arg ) {
	int n = arg->size();
	std::string ext = arg->substr(n-4,n);
	if (ext == ".su2")
		return SU2;
	else if (ext == ".vtk")
		return VTK;
	else if (ext == ".xyz" || ext == ".p3d")
		return PLOT3D;
	else
		return UNKNOWN;
}

int main (int argc, char* argv[])
{
	if (argc < 2) {
		Fatal("One argument required");
	}
	int i = 1;
	std::string *arg, * inputfile = NULL, *outputfile = NULL, *translationfile = NULL;
	bool mergepoints;
	while (i < argc) {
		arg = new std::string(argv[i]);
		if (arg->at(0) == '-') {
			if (*arg == "-t") {
				i++;
				if (i == argc) Fatal("Must pass filename option to -t");
				arg = new std::string(argv[i]);
				translationfile = arg;
			} else if (*arg == "-o") {
				i++;
				if (i == argc) Fatal("Must pass filename option to -o");
				arg = new std::string(argv[i]);
				outputfile = arg;
			} else if (*arg == "-m") {
				mergepoints = true;
			}
		} else {
			if (inputfile) Fatal("blockname defined twice");
			inputfile = arg;
		}
		i++;
	}
	if (!inputfile) {
		Fatal("Must define input file");
	} else if (!outputfile) {
		Fatal("Must define output file");
	}
	Grid * grid;
	MultiBlock *mb;
	switch (get_blocktype(inputfile)) {
		case PLOT3D:
			mb = ReadPlot3D(inputfile);
			grid = to_grid(mb);
			break;
		case SU2:
			grid = readSU2(inputfile);
			break;
		case VTK:
			Fatal("Input file not supported");
			break;
		default:
			Fatal("Input file not recognized");
			break;
	}
	set_i_points(grid);
	if (mergepoints) {
		sort_points_by_location(grid);
		merge_points(grid,TOL);
		sort_points_by_index(grid);
		set_i_elements(grid);
		sort_elements(grid);
		delete_inner_faces(grid);
		collapse_elements(grid);
		sort_elements_by_index(grid);
	}
	if (translationfile) {
		TranslationTable * transt = new TranslationTable(grid->names.size());
		ReadTranslationFile(translationfile,transt);
		applyTranslation(grid,transt);
	}
	switch (get_blocktype(outputfile)) {
		case PLOT3D:
			Fatal("Output file not supported");
			break;
		case SU2:
			toSU2(outputfile,grid);
			break;
		case VTK:
			toVTK(outputfile,grid);
			break;
		default:
			Fatal("Output file not recognized");
			break;
	}
	return 0;
}
