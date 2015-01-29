#include <iostream>
#include <string>
#include <cstdlib>

#include "error.h"
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

int get_blocktype ( std::string &arg ) {
	int n = arg.size();
	std::string ext = arg.substr(n-4,n);
	if (ext == ".su2")
		return SU2;
	else if (ext == ".vtk")
		return VTK;
	else if (ext == ".xyz" || ext == ".p3d")
		return PLOT3D;
	else
		return UNKNOWN;
}

void print_usage () {
	std::cerr << "unstruc [-m] [-t translation_file] output_file input_file [input_file ...]" << std::endl << std::endl;
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
	char * c_outputfile = NULL;
	char * c_translationfile = NULL;
	std::string arg;
	std::vector <std::string> inputfiles;
	bool mergepoints;
	while (i < argc) {
		if (argv[i][0] == '-') {
			std::string arg (argv[i]);
			if (arg == "-t") {
				i++;
				if (i == argc) Fatal("Must pass filename option to -t");
				c_translationfile = argv[i];
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
				c_outputfile = argv[i];
			else if (j > 0)
				inputfiles.emplace_back(argv[i]);
			else {
				print_usage();
				Fatal("Specify one input_file and one output_file");
			}
			j++;
		}
		i++;
	}
	if (!c_outputfile) {
		print_usage();
		Fatal("Must specify output file");
	}
	if (inputfiles.size() == 0) {
		print_usage();
		Fatal("Must specify input file[s]");
	}
	std::string outputfile (c_outputfile);
	Grid grid;
	MultiBlock mb;
	switch (get_blocktype(inputfiles[0])) {
		case PLOT3D:
			readPlot3D(mb,inputfiles[0]);
			to_grid(grid,mb);
			break;
		case SU2:
			readSU2(grid,inputfiles[0]);
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
		sort_elements_by_index(grid);

		collapse_elements(grid);
	}
	if (c_translationfile) {
		std::string translationfile (c_translationfile);
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
