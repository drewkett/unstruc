#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "unstruc.h"

#define TOL 3.e-8

using namespace unstruc;

struct TranslationTable {
  std::vector <Name> names;
  std::vector <bool> translate;
  std::vector <size_t> index;
  TranslationTable(size_t n) : names(0), index(n), translate(n) {};
};

void ReadTranslationFile(std::string &filename, TranslationTable &tt) {
  std::ifstream f;
  std::string line, s;
  std::cerr << "Reading Translation File '" << filename << "'" << std::endl;
  f.open(filename.c_str(),std::ios::in);
  if (!f.is_open()) fatal("Could not open file");
  while (getline(f,line)) {
    tt.names.resize(tt.names.size() + 1);
    Name &name = tt.names.back();
    name.dim = 2;
    std::istringstream iss(line);
    iss >> name.name;
    std::cerr << name.name;
    while (! iss.eof()) {
      iss >> s;
      std::cerr << " " << s;
      size_t i = atoi(s.c_str());
      tt.index[i] = tt.names.size()-1;
      tt.translate[i] = true;
    }
    std::cerr << std::endl;
  }
  f.close();
}

void applyTranslation(Grid &grid, TranslationTable &transt) {
  size_t offset = grid.names.size();
  for (size_t i=0; i < transt.names.size(); i++) {
    grid.names.push_back(transt.names[i]);
  }
  for (size_t i=0; i < transt.index.size(); i++) {
    if (transt.translate[i]) {
      transt.index[i] += offset;
    }
  }
  for (size_t i = 0; i < grid.elements.size(); i++) {
    Element &e = grid.elements[i];
    if (e.name_i != -1 && transt.translate[e.name_i]) e.name_i = transt.index[e.name_i];
  }
}

void print_usage () {
  std::cerr <<
    "unstruc-convert [options] output_file input_file [input_file ...]\n\n"
    "This tool converts between file formats typically used in CFD analysis. Currently supported input file types are Plot3D (.xyz or .p3d) and SU2 (.su2). Currently supported output file types are SU2 (.su2) and VTK (.vtk)\n"
    "Option Arguments\n"
    "-m                   Attempt to merge points that are close together\n"
    "-s scale_factor      Scale model by a factor\n"
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
  double scale_factor = 1;
  while (i < argc) {
    if (argv[i][0] == '-') {
      std::string arg (argv[i]);
      if (arg == "-t") {
        i++;
        if (i == argc) fatal("Must pass filename option to -t");
        c_translationfile = argv[i];
      } else if (arg == "-m") {
        mergepoints = true;
      } else if (arg == "-s") {
        i++;
        if (i == argc) fatal("Must pass float option to -s");
        scale_factor = atof(argv[i]);
      } else if (arg == "-h" || arg == "--help") {
        print_usage();
        exit(0);
      } else {
        print_usage();
        fatal("Unknown argument");
      }
    } else {
      if (j == 0)
        c_outputfile = argv[i];
      else
        inputfiles.push_back( std::string(argv[i]) );
      j++;
    }
    i++;
  }
  if (!c_outputfile) {
    print_usage();
    fatal("Must specify output file");
  }
  if (inputfiles.size() == 0) {
    print_usage();
    fatal("Must specify input file[s]");
  }
  std::string outputfile (c_outputfile);
  Grid grid;
  for (size_t i = 0; i < inputfiles.size(); ++i) {
    grid = read_grid(inputfiles[i]);
  }
  if (scale_factor != 1)
    fprintf(stderr,"Scaling mesh by %gx\n",scale_factor);
  for (Point& p : grid.points) {
    p.x = p.x*scale_factor;
    p.y = p.y*scale_factor;
    p.z = p.z*scale_factor;
  }
  //set_i_points(grid);
  if (mergepoints) {
    grid.merge_points(TOL);
    grid.delete_inner_faces();
    grid.collapse_elements(false);
  }
  if (c_translationfile) {
    std::string translationfile (c_translationfile);
    TranslationTable transt = TranslationTable(grid.names.size());
    ReadTranslationFile(translationfile,transt);
    applyTranslation(grid,transt);
  }
  write_grid(outputfile,grid);
  return 0;
}
