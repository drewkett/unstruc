#include "su2.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

namespace unstruc {

  bool su2_write(const std::string& outputfile, const Grid& grid) {
    size_t i, j;

    FILE * f;
    f = fopen(outputfile.c_str(),"w");
    if (!f) fatal("Could not open file");
    std::cerr << "Outputting SU2" << std::endl;
    std::cerr << "Writing Elements" << std::endl;
    fprintf(f,"NDIME= %d\n\n",grid.dim);
    size_t n_volume_elements = 0;
    for (const Element& e : grid.elements)
      if (Shape::Info[e.type].dim == grid.dim)
        n_volume_elements++;

    fprintf(f,"NELEM= %d\n",n_volume_elements);
    for (const Element& e : grid.elements) {
      if (Shape::Info[e.type].dim != grid.dim) continue;
      fprintf(f,"%d",Shape::Info[e.type].vtk_id);
      for (size_t p : e.points) {
        assert (p < grid.points.size());
        fprintf(f," %d",p);
      }
      fprintf(f,"\n");
    }
    fprintf(f,"\n");
    std::cerr << "Writing Points" << std::endl;
    fprintf(f,"NPOIN= %zu\n",grid.points.size());
    for (i = 0; i < grid.points.size(); i++) {
      const Point& p = grid.points[i];
      if (grid.dim == 2)
        fprintf(f,"%.17g %.17g %d\n",p.x,p.y,i);
      else
        fprintf(f,"%.17g %.17g %.17g %d\n",p.x,p.y,p.z,i);
    }
    fprintf(f,"\n");
    std::vector<size_t> name_count(grid.names.size(),0);
    for (i = 0; i < grid.elements.size(); i++) {
      const Element &e = grid.elements[i];
      if (Shape::Info[e.type].dim != (grid.dim-1)) continue;
      if (e.name_i == -1) continue;
      name_count[e.name_i]++;
    }
    size_t n_names = 0;
    for (i = 0; i < grid.names.size(); i++) {
      if (name_count[i] > 0)
        n_names++;
    }
    std::cerr << "Writing Markers" << std::endl;
    fprintf(f,"NMARK= %d\n",n_names);
    for (i = 0; i < grid.names.size(); i++) {
      if (name_count[i] == 0) continue;
      const Name& name = grid.names[i];
      if (name.dim != grid.dim - 1) continue;
      std::cerr << i << " : " << name.name << " (" << name_count[i] << ")" << std::endl;
      fprintf(f,"MARKER_TAG= %s\n",name.name.c_str());
      fprintf(f,"MARKER_ELEMS= %d\n",name_count[i]);
      for (j = 0; j < grid.elements.size(); j++) {
        const Element &e = grid.elements[j];
        if (Shape::Info[e.type].dim != grid.dim - 1) continue;
        if (e.name_i == -1) continue;
        fprintf(f,"%d",Shape::Info[e.type].vtk_id);
        for (size_t p : e.points) {
          assert (p < grid.points.size());
          fprintf(f," %d",p);
        }
        fprintf(f,"\n");
      }
    }
    fprintf(f,"\n");
    fclose(f);
    return true;
  }

  Grid su2_read(const std::string& inputfile) {
    Grid grid;
    size_t ipoint, iname, i, j, k;
    size_t nelem;
    Name name;
    std::ifstream f;
    std::istringstream ls;
    std::string line, token;
    std::map<size_t,size_t> point_map;
    bool use_point_map = false;
    f.open(inputfile.c_str(),std::ios::in);
    std::cerr << "Opening SU2 File '" << inputfile << "'" << std::endl;
    if (!f.is_open()) fatal("Could not open file");
    bool read_dime = false, read_poin = false, read_elem = false;
    while (getline(f,line)) {
      std::stringstream ss(line);
      ss >> token;
      if (!ss) continue;
      if (token.substr(0,6) == "NDIME=") {
        read_dime = true;
        if (token.size() > 6)
          grid.dim = std::atoi(token.substr(6).c_str());
        else
          ss >> grid.dim;
        std::cerr << grid.dim << " Dimensions" << std::endl;

        //Create default named block to be assigned to all elements
        name = Name();
        name.name.assign("default");
        name.dim = grid.dim;
        grid.names.push_back(name);
      } else if (token.substr(0,6) == "NELEM=") {
        read_elem = true;
        size_t n_elems = 0;
        if (token.size() > 6)
          n_elems = std::atoi(token.substr(6).c_str());
        else
          ss >> n_elems;
        std::cerr << n_elems << " Elements" << std::endl;
        grid.elements.reserve(n_elems);
        for (size_t i = 0; i < n_elems; ++i) {
          getline(f,line);
          std::stringstream ss(line);
          size_t vtk_id;
          ss >> vtk_id;
          Shape::Type type = type_from_vtk_id(vtk_id);
          if (type == Shape::Undefined) fatal("Unrecognized shape type");
          Element elem = Element(type);
          //Assign to default name block
          elem.name_i = 0;
          for (j = 0; j < elem.points.size(); ++j) {
            ss >> ipoint;
            elem.points[j] = ipoint;
          }
          grid.elements.push_back(elem);
        }
      } else if (token.substr(0,6) == "NPOIN=") {
        read_poin = true;
        size_t n_points = 0;
        if (token.size() > 6)
          n_points = std::atoi(token.substr(6).c_str());
        else
          ss >> n_points;

        grid.points.resize(n_points);
        ss >> n_points;

        if (n_points > grid.points.size())
          grid.points.resize(n_points);

        std::cerr << n_points << " Points" << std::endl;

        for (i = 0; i < n_points; ++i) {
          getline(f,line);
          std::stringstream ss(line);
          Point point;
          ss >> point.x;
          ss >> point.y;
          if (!grid.dim)
            fatal("Dimension (NDIME) not defined");
          if (grid.dim == 3)
            ss >> point.z;

          if (!ss.eof()) {
            ss >> ipoint;
            use_point_map = true;
          } else {
            ipoint = i;
          }
          point_map[ipoint] = i;
          grid.points[i] = point;
        }
      } else if (token.substr(0,6) == "NMARK=") {
        size_t nmark;
        if (token.size() > 6)
          nmark = std::atoi(token.substr(6).c_str());
        else
          ss >> nmark;

        std::cerr << nmark << " Markers" << std::endl;
        for (i = 0; i < nmark; i++) {
          name = Name();
          name.dim = grid.dim-1;
          iname = grid.names.size();

          // Read MARKER_TAG=
          getline(f,line);
          std::stringstream ss(line);
          ss >> token;
          if (token.substr(0,11) != "MARKER_TAG=")
            fatal("Invalid Marker Definition: Expected MARKER_TAG=");
          if (token.size() > 11) {
            name.name.assign(token.substr(11));
          } else {
            ss >> token;
            name.name.assign(token);
          }
          grid.names.push_back(name);
          std::cerr << name.name << std::endl;

          // Read MARKER_ELEMS=
          getline(f,line);
          ss.clear();
          ss.str(line);
          ss >> token;
          if (token.substr(0,13) != "MARKER_ELEMS=")
            fatal("Invalid Marker Definition: Expected MARKER_ELEMS=");
          if (token.size() > 13)
            nelem = std::atoi(token.substr(13).c_str());
          else
            ss >> nelem;
          for (j = 0; j < nelem; j++) {
            getline(f,line);
            ss.clear();
            ss.str(line);
            size_t vtk_id;
            ss >> vtk_id;
            Shape::Type type = type_from_vtk_id(vtk_id);
            if (type == Shape::Undefined) fatal("Unrecognized shape type");
            Element elem = Element(type);
            for (k=0; k<elem.points.size(); k++) {
              ss >> ipoint;
              if (ipoint >= grid.points.size()) fatal("Error Marker Element");
              elem.points[k] = ipoint;
            }
            elem.name_i = iname;
            grid.elements.push_back(elem);
          }
        }
      }
    }
    assert (read_dime);
    assert (read_elem);
    assert (read_poin);
    size_t n_negative = 0;
    for (Element& e : grid.elements) {
      if (use_point_map) {
        for (size_t& p : e.points) {
          assert (point_map.count(p) == 1);
          p = point_map[p];
        }
      }
      if (e.calc_volume(grid) < 0) {
        if (e.type == Shape::Tetra) {
          std::swap(e.points[1],e.points[2]);
          assert (e.calc_volume(grid) > 0);
        }
        n_negative++;
      }
    }
    if (n_negative)
      fprintf(stderr,"%d Negative Volume Elements\n",n_negative);
    return grid;
  }

} // namespace unstruc::su2
