#include <iostream>
#include <string>
#include <cstdlib>

#include "su2.h"
#include "vtk.h"
#include "gmsh.h"
#include "openfoam.h"
#include "plot3d.h"

#include "error.h"
#include "block.h"
#include "translation.h"

#define TOL 3.e-8

enum BLOCKTYPE {
	UNKNOWN = 0,
	PLOT3D = 1,
	SU2 = 2,
	VTK = 3,
	OPENFOAM = 4
};

int get_blocktype ( std::string &arg ) {
	int n = arg.size();
	std::string ext = arg.substr(n-4,n);
	if (arg.compare(n-4,4,".su2") == 0)
		return SU2;
	else if (arg.compare(n-4,4,".vtk") == 0)
		return VTK;
	else if (arg.compare(n-4,4,".xyz") == 0 || arg.compare(n-4,4,".p3d") == 0)
		return PLOT3D;
	else if (arg.compare(n-5,5,".foam") == 0)
		return OPENFOAM;
	else
		return UNKNOWN;
}

void print_usage () {
	std::cerr << "unstruc [-m] [-t translation_file] input_file output_file" << std::endl << std::endl;
	std::cerr << "This tool converts between file formats typically used in CFD analysis. Currently supported input file types are Plot3D (.xyz or .p3d) and SU2 (.su2). Currently supported output file types are SU2 (.su2) and VTK (.vtk)" << std::endl << std::endl;
	std::cerr << "Option Arguments" << std::endl;
	std::cerr << "-m                   Attempt to merge points that are close together" << std::endl;
	std::cerr << "-t translation_file  Specify translation file for changing surface/block names" << std::endl;
	std::cerr << "-h, --help           Print usage" << std::endl;
	std::cerr << std::endl;
}
int main (int argc, char* argv[])
{
	int i = 1, j = 0;
	std::string arg, outputfile, inputfile, translationfile;
	bool mergepoints;
	while (i < argc) {
		arg.assign(argv[i]);
		if (arg.at(0) == '-') {
			if (arg == "-t") {
				i++;
				if (i == argc) Fatal("Must pass filename option to -t");
				translationfile.assign(argv[i]);
			} else if (arg == "-m") {
				mergepoints = true;
			} else if (arg == "-h" || arg == "--help") {
				print_usage();
				exit(0);
			} else {
				print_usage();
				Fatal("Unknown argument");
			}
		} else {
			if (j == 0)
				inputfile.assign(arg);
			else if (j == 1)
				outputfile.assign(arg);
			else {
				print_usage();
				Fatal("Specify one input_file and one output_file");
			}
			j++;
		}
		i++;
	}
	if (inputfile.empty()) {
		print_usage();
		Fatal("Must specify input and output file");
	} else if (outputfile.empty()) {
		print_usage();
		Fatal("Must specify output file");
	}
	Grid grid;
	MultiBlock mb;
	switch (get_blocktype(inputfile)) {
		case PLOT3D:
			readPlot3D(mb,inputfile);
			to_grid(grid,mb);
			break;
		case SU2:
			readSU2(grid,inputfile);
			break;
		case OPENFOAM:
			readOpenFoam(grid,inputfile);
			return 0;
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
		sort_elements_by_index(grid);

		collapse_elements(grid);
	}
	if (!translationfile.empty()) {
		TranslationTable transt = TranslationTable(grid.names.size());
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
