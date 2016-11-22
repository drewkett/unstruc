#include "unstruc.h"

using namespace unstruc;

void print_usage () {
  fprintf(stderr,
          "unstruc-quality [options] mesh_file\n"
          "-b bad_elements_filename  Output bad elements to a file\n"
          "-t angle_threshold        Set small angle threshold to identify bad elements (Default=1)\n"
          "-h                                Print Usage\n");
}

int parse_failed (std::string msg) {
  print_usage();
  fprintf(stderr,"\n%s\n",msg.c_str());
  return 1;
}

int main(int argc, char* argv[]) {
  int argnum = 0;
  std::string filename, bad_elements_filename;
  double angle_threshold = 1;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg (argv[i]);
      if (arg == "-b") {
        ++i;
        if (i == argc) return parse_failed("Must pass filename to -b");
        bad_elements_filename = std::string (argv[i]);
      } else if (arg == "-t") {
        ++i;
        if (i == argc) return parse_failed("Must pass float to -t");
        angle_threshold = std::stof(argv[i]);
      } else {
        return parse_failed("Unknown option passed '"+arg+"'");
      }
    } else {
      argnum++;
      if (argnum == 1) {
        filename = std::string(argv[i]);
      } else {
        return parse_failed("Extra argument passed");
      }
    }
  }
  if (filename.empty())
    return parse_failed("Must pass mesh filename");

  Grid mesh = read_grid(filename);
  MeshQuality quality = get_mesh_quality(mesh, angle_threshold);
  printf("Face Angle     : %7.3f %7.3f\n",quality.face_angle.min,quality.face_angle.max);
  if (quality.dihedral_angle.min != 180 || quality.dihedral_angle.max != 0)
    printf("Dihedral Angle : %7.3f %7.3f\n",quality.dihedral_angle.min,quality.dihedral_angle.max);
  if (quality.bad_elements.size()) {
    printf("%lu Bad Elements\n",quality.bad_elements.size());
    if (!bad_elements_filename.empty()) {
      Grid bad = mesh.grid_from_element_index(quality.bad_elements);
      write_grid(bad_elements_filename, bad);
    }
  }
}
