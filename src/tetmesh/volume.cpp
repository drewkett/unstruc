#include "volume.h"

#include "unstruc.h"

#include <tetgen.h>

using namespace unstruc;

namespace tetmesh {

  Grid grid_from_tetgenio(tetgenio const& tg) {
    Grid grid (3);
    grid.points.reserve(tg.numberofpoints);
    for (int i = grid.points.size(); i < tg.numberofpoints; ++i) {
      Point p;
      p.x = tg.pointlist[3*i];
      p.y = tg.pointlist[3*i+1];
      p.z = tg.pointlist[3*i+2];
      grid.points.push_back(p);
    }
    grid.elements.reserve(grid.elements.size()+tg.numberoftetrahedra);
    for (int i = 0; i < tg.numberoftetrahedra; ++i) {
      Element e (Shape::Tetra);
      assert (tg.numberofcorners == 4);
      e.points[0] = tg.tetrahedronlist[4*i];
      e.points[1] = tg.tetrahedronlist[4*i+1];
      e.points[2] = tg.tetrahedronlist[4*i+2];
      e.points[3] = tg.tetrahedronlist[4*i+3];
      grid.elements.push_back(e);
    }
    return grid;
  }

  Grid volgrid_from_surface(Grid const& surface, const std::vector<Point>& holes, double min_ratio) {
    tetgenio in;
    in.mesh_dim = 3;
    in.firstnumber = 0;

    in.numberofpoints = surface.points.size();
    in.pointlist  = new REAL[surface.points.size()*3];
    size_t i = 0;
    for (Point const& p : surface.points) {
      in.pointlist[i] = p.x;
      in.pointlist[i+1] = p.y;
      in.pointlist[i+2] = p.z;
      i += 3;
    }

    in.numberoffacets = surface.elements.size();
    in.facetlist = new tetgenio::facet[in.numberoffacets];
    in.facetmarkerlist = new int[in.numberoffacets];

    for (size_t i = 0; i < surface.elements.size(); ++i) {
      Element const& e = surface.elements[i];
      tetgenio::facet& f = in.facetlist[i];
      tetgenio::init(&f);

      in.facetmarkerlist[i] = e.name_i;

      f.numberofpolygons = 1;
      f.polygonlist = new tetgenio::polygon[1];
      tetgenio::polygon& p = f.polygonlist[0];
      tetgenio::init(&p);
      if (e.type != Shape::Triangle)
        not_implemented("Currently only supports triangles");

      p.numberofvertices = e.points.size();
      p.vertexlist = new int[e.points.size()];
      p.vertexlist[0] = e.points[0];
      p.vertexlist[1] = e.points[1];
      p.vertexlist[2] = e.points[2];
    }

    in.numberofholes = holes.size();
    in.holelist = new REAL[in.numberofholes*3];
    for (size_t i = 0; i < holes.size(); ++i) {
      const Point& hole = holes[i];
      in.holelist[3*i]   = hole.x;
      in.holelist[3*i+1] = hole.y;
      in.holelist[3*i+2] = hole.z;
    }
    tetgenbehavior tg;
    tg.plc = 1;
    tg.nobisect = 1;
    tg.quiet = 1;
    tg.quality = 1;
    if (min_ratio > 1)
      tg.minratio = min_ratio;
    //tg.mindihedral = 0;
    //tg.verbose = 1;

    tetgenio out;
    tetrahedralize(&tg,&in,&out,NULL,NULL);

    return grid_from_tetgenio(out);
  }

  Grid volgrid_from_surface(Grid const& surface) {
    std::vector <Point> holes;
    double min_ratio = 0;
    return volgrid_from_surface(surface,holes,min_ratio);
  }

  Point find_point_inside_surface(const Grid& surface) {
    Grid vol = volgrid_from_surface(surface);

    const Element& e = surface.elements[0];
    assert (e.type == Shape::Triangle);
    const Point& p0 = surface.points[e.points[0]];
    const Point& p1 = surface.points[e.points[1]];
    const Point& p2 = surface.points[e.points[2]];
    Vector v1 = p1 - p0;
    Vector v2 = p2 - p1;
    Vector n = cross(v1,v2);
    Point test = p0 + n*1e-6;
    Point test2 = p0 - n*1e-6;

    if (vol.test_point_inside(test))
      return test;
    else if (vol.test_point_inside(test2))
      return test2;
    else
      fatal("Not sure what to do");

    return Point { 0, 0, 0 };
  }

  Point orient_surface(Grid& surface) {
    Grid vol = volgrid_from_surface(surface);

    bool found_point = false;
    for (const Element& e : surface.elements) {
      assert (e.type == Shape::Triangle);
      const Point& p0 = surface.points[e.points[0]];
      const Point& p1 = surface.points[e.points[1]];
      const Point& p2 = surface.points[e.points[2]];
      Point p;
      p.x = (p0.x + p1.x + p2.x)/3;
      p.y = (p0.y + p1.y + p2.y)/3;
      p.z = (p0.z + p1.z + p2.z)/3;
      Vector v1 = p1 - p0;
      Vector v2 = p2 - p1;
      Vector n = cross(v1,v2);
      Point test = p - n*1e-6;
      Point test2 = p + n*1e-6;

      if (vol.test_point_inside(test))
        return test;
      else if (vol.test_point_inside(test2)) {
#ifndef NDEBUG
        fprintf(stderr,"(tetmesh::orient_surface) Reorienting Surface\n");
#endif
        for (Element& e : surface.elements) {
          if (e.type != Shape::Triangle)
            fatal("(tetmesh::orient_surface) current only works with triangle surfaces");
          std::swap(e.points[1],e.points[2]);
        }
        return test2;
      }
    }
    fatal("Neither guessed point is inside surface. Hmmm");
    return Point { 0, 0, 0 };
  }
}
