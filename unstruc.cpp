#include <iostream>
#include <string>
#include <cstdlib>

#include "stl.h"
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
	OPENFOAM = 4,
	STL = 5
};

int get_blocktype ( std::string &arg ) {
	int n = arg.size();
	std::string ext = arg.substr(n-4,n);
	if (arg.compare(n-4,4,".su2") == 0)
		return SU2;
	else if (arg.compare(n-4,4,".stl") == 0)
		return STL;
	else if (arg.compare(n-4,4,".vtk") == 0)
		return VTK;
	else if (arg.compare(n-4,4,".xyz") == 0 || arg.compare(n-4,4,".p3d") == 0)
		return PLOT3D;
	else if (arg.compare(n-8,8,"polyMesh") == 0 || arg.compare(n-9,9,"polyMesh/") == 0)
		return OPENFOAM;
	else
		return UNKNOWN;
}

void print_usage () {
	std::cerr << 
"unstruc [-m] [-t translation_file] output_file input_file [input_file ...]\n\n"
"This tool converts between file formats typically used in CFD analysis. Currently supported input file types are Plot3D (.xyz or .p3d) and SU2 (.su2). Currently supported output file types are SU2 (.su2) and VTK (.vtk)\n"
"Option Arguments\n"
"-m                   Attempt to merge points that are close together\n"
"-t translation_file  Specify translation file for changing surface/block names\n"
"-h, --help           Print usage\n";
}
int main (int argc, char* argv[])
{
	int i = 1, j = 0;
	char * c_outputfile = NULL;
	char * c_translationfile = NULL;
	std::string arg;
	std::vector <std::string> inputfiles;
	bool mergepoints = false;
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
			else 
				inputfiles.emplace_back(argv[i]);
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
	for (int i = 0; i < inputfiles.size(); ++i) {
		switch (get_blocktype(inputfiles[i])) {
			case PLOT3D:
				readPlot3DToGrid(grid,inputfiles[i]);
				break;
			case SU2:
				grid = readSU2(inputfiles[i]);
				break;
			case STL:
				grid = read_stl_ascii(inputfiles[i]);
				break;
			case VTK:
				Fatal("Input file not supported");
				break;
			case OPENFOAM:
				grid = readOpenFoam(inputfiles[i]);
				break;
			default:
				Fatal("Input file not recognized");
				break;
		}
	}
	//set_i_points(grid);
	if (mergepoints) {
		grid.merge_points(TOL);
		grid.delete_inner_faces();
		grid.collapse_elements();
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
