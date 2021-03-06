#include "vtk.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

namespace unstruc {

  bool vtk_write(const std::string& filename, const Grid &grid) {
    FILE * f;
    f = fopen(filename.c_str(),"w");
    std::cerr << "Writing " << filename << std::endl;
    if (!f) fatal("Could not open file");
    fprintf(f,"# vtk DataFile Version 2.0\n");
    fprintf(f,"Description\n");
    fprintf(f,"ASCII\n");
    fprintf(f,"DATASET UNSTRUCTURED_GRID\n");
    //std::cerr << "Writing Points" << std::endl;
    fprintf(f,"POINTS %d double\n",grid.points.size());
    for (const Point& p : grid.points) {
      fprintf(f,"%.17g %.17g",p.x,p.y);
      if (grid.dim == 3)
        fprintf(f," %.17g\n",p.z);
      else
        fprintf(f," 0.0\n");
    }
    //std::cerr << "Writing Cells" << std::endl;
    size_t n_volume_elements = 0;
    size_t n_elvals = 0;
    for (const Element& e : grid.elements) {
      n_volume_elements++;
      n_elvals += e.points.size()+1;
    }
    fprintf(f,"CELLS %d %d\n",n_volume_elements,n_elvals);
    for (const Element& e : grid.elements) {
      fprintf(f,"%d",e.points.size());
      for (size_t p : e.points) {
        fprintf(f," %d",p);
      }
      fprintf(f,"\n");
    }

    fprintf(f,"CELL_TYPES %d\n",n_volume_elements);
    for (const Element& e : grid.elements) {
      fprintf(f,"%d\n",Shape::Info[e.type].vtk_id);
    }
    fclose(f);
    return true;
  }

  void vtk_write_point_data_header(const std::string& filename, const Grid& grid) {
    std::ofstream f (filename, std::ios::app);
    if (!f.is_open()) fatal("Could not open file");
    f << std::endl << "POINT_DATA " << grid.points.size() << std::endl;
  }

  void vtk_write_cell_data_header(const std::string& filename, const Grid& grid) {
    std::ofstream f (filename, std::ios::app);
    if (!f.is_open()) fatal("Could not open file");
    f << std::endl << "CELL_DATA " << grid.points.size() << std::endl;
  }

  void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <size_t>& scalars) {
    std::ofstream f (filename, std::ios::app);
    if (!f.is_open()) fatal("Could not open file");
    f << "SCALARS " << name << " int 1" << std::endl;
    f << "LOOKUP_TABLE" << std::endl;
    for (size_t s : scalars) {
      f << s << std::endl;
    }
    f << std::endl;
  }

  void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <double>& scalars) {
    std::ofstream f (filename, std::ios::app);
    if (!f.is_open()) fatal("Could not open file");
    f.precision(15);
    f << "SCALARS " << name << " double 1" << std::endl;
    f << "LOOKUP_TABLE default" << std::endl;
    for (double s : scalars) {
      f << s << std::endl;
    }
    f << std::endl;
  }

  void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <Vector>& vectors) {
    std::ofstream f (filename, std::ios::app);
    if (!f.is_open()) fatal("Could not open file");
    f.precision(15);
    f << "Vectors " << name << " double" << std::endl;
    for (const Vector& v : vectors) {
      f << v.x << " " << v.y << " " << v.z << std::endl;
    }
    f << std::endl;
  }

  std::unique_ptr<Grid> vtk_read_ascii(std::ifstream& f) {
    std::unique_ptr<Grid> grid (new Grid (3));
    std::string token;
    f >> token;
    if (token != "DATASET") return nullptr;

    f >> token;
    if (token != "UNSTRUCTURED_GRID") return nullptr;

    f >> token;
    if (token != "POINTS") return nullptr;

    size_t n_points;
    f >> n_points;
    if (!f) return nullptr;

    f >> token;
    if (token != "double") return nullptr;

    grid->points.reserve(n_points);
    for (size_t i = 0; i < n_points; ++i) {
      Point p;
      f >> p.x;
      f >> p.y;
      f >> p.z;
      grid->points.push_back(p);
    }
    if (!f) return nullptr;

    f >> token;
    if (token != "CELLS") return nullptr;

    size_t n_cells, n_cells_size;
    f >> n_cells;
    f >> n_cells_size;
    if (!f) return nullptr;

    std::vector<size_t> cells (n_cells_size);
    for (size_t i = 0; i < n_cells_size; ++i) {
      f >> cells[i];
    }

    f >> token;
    if (token != "CELL_TYPES") return nullptr;

    size_t n_cells2;
    f >> n_cells2;
    if (!f || n_cells != n_cells2) return nullptr;

    grid->elements.reserve(n_cells);

    size_t cj = 0;
    for (size_t i = 0; i < n_cells; ++i) {
      size_t vtk_id;
      f >> vtk_id;
      Shape::Type type = type_from_vtk_id(vtk_id);

      size_t n_elem_points = cells[cj];
      cj++;

      if (Shape::Info[type].n_points && Shape::Info[type].n_points != n_elem_points)
        return nullptr;

      Element e (type);
      if (Shape::Info[type].n_points == 0)
        e.points.resize(n_elem_points);

      for (size_t j = 0; j < n_elem_points; ++j) {
        e.points[j] = cells[cj];
        cj++;
      }
      grid->elements.push_back(e);
    }
    if (!f) return nullptr;
    if (cj != n_cells_size) return nullptr;

    return std::move(grid);
  }

  Grid vtk_read(const std::string& filename) {
    std::cerr << "Reading " << filename << std::endl;
    std::ifstream f (filename);
    if (!f.is_open()) fatal("Could not open file");

    char line[256];
    f.getline(line,256);
    if (strncmp(line,"# vtk DataFile Version 2.0",256) != 0)
      fatal("Invalid VTK File : " + filename);
    f.getline(line,256);
    std::string type;
    f >> type;
    std::unique_ptr<Grid> grid;
    if (type == "ASCII")
      grid = vtk_read_ascii(f);
    else if (type == "BINARY")
      not_implemented("Currently don't support binary vtk files");

    if (!grid)
      fatal("Error reading VTK file : "+filename);
    return *grid;
  }

} // namespace unstruc
