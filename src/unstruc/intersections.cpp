#include "intersections.h"

#include "point.h"
#include "grid.h"
#include "error.h"
#include "io.h"

#include <array>
#include <cfloat>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <cfloat>
#include <memory>
#include <bitset>
#include <iostream>
#include <utility>

namespace unstruc {

  const size_t max_depth = 10;
  const size_t max_bin_size = 20;

  struct Edge {
    size_t p1, p2;
    std::vector <size_t> elements;
    Point min, max;

    Edge() {};
    Edge(size_t _p1,size_t _p2) {
      if (_p1 < _p2) {
        p1 = _p1;
        p2 = _p2;
      } else {
        p1 = _p2;
        p2 = _p1;
      }
    };

    Edge(size_t _p1,size_t _p2,size_t e) {
      if (_p1 < _p2) {
        p1 = _p1;
        p2 = _p2;
      } else {
        p1 = _p2;
        p2 = _p1;
      }
      elements.push_back(e);
    };

    bool operator==(const Edge& other) const {
      return (p1 == other.p1) && (p2 == other.p2);
    }
    bool operator!=(const Edge& other) const {
      return !(*this == other);
    }
    bool operator<(const Edge& other) const {
      if (p1 == other.p1)
        return p2 < other.p2;
      else
        return p1 < other.p1;
    }

    static bool compare_by_min_x(const Edge& e1, const Edge& e2) {
      return e1.min.x < e2.min.x;
    }
  };

  struct Face {
    std::vector<size_t> points;
    std::vector <size_t> elements;

    Point center;
    Vector normal;
    Point min, max;

    bool operator==(const Face& other) const {
      return points == other.points;
    }
    bool operator<(const Face& other) const {
      return points < other.points;
    }

    static bool compare_by_min_x(const Face& f1, const Face& f2) {
      return f1.min.x < f2.min.x;
    }
  };

  std::vector<Edge> get_edges(const Grid& grid) {
    std::vector<Edge> edges;
#ifndef NDEBUG
    fprintf(stderr,"Create Edges\n");
#endif
    size_t n_edges = 0;
    for (const Element& e : grid.elements) {
      size_t n = Shape::Info[e.type].n_edges;
      if (n == 0) fatal();
      n_edges += n;
    }
    edges.reserve(n_edges);

    for (size_t _e = 0; _e < grid.elements.size(); ++_e) {
      const Element& e = grid.elements[_e];
      if (e.type == Shape::Wedge) {
        edges.push_back( Edge(e.points[0],e.points[1],_e) );
        edges.push_back( Edge(e.points[1],e.points[2],_e) );
        edges.push_back( Edge(e.points[2],e.points[0],_e) );

        edges.push_back( Edge(e.points[3],e.points[4],_e) );
        edges.push_back( Edge(e.points[4],e.points[5],_e) );
        edges.push_back( Edge(e.points[5],e.points[3],_e) );

        edges.push_back( Edge(e.points[0],e.points[3],_e) );
        edges.push_back( Edge(e.points[1],e.points[4],_e) );
        edges.push_back( Edge(e.points[2],e.points[5],_e) );
      } else if (e.type == Shape::Triangle) {
        edges.push_back( Edge(e.points[0],e.points[1],_e) );
        edges.push_back( Edge(e.points[1],e.points[2],_e) );
        edges.push_back( Edge(e.points[2],e.points[0],_e) );
      } else if (e.type == Shape::Quad) {
        edges.push_back( Edge(e.points[0],e.points[1],_e) );
        edges.push_back( Edge(e.points[1],e.points[2],_e) );
        edges.push_back( Edge(e.points[2],e.points[3],_e) );
        edges.push_back( Edge(e.points[3],e.points[0],_e) );
      } else
        fatal("Shape not supported");

    }
#ifndef NDEBUG
    fprintf(stderr,"Sorting Faces\n");
#endif
    std::sort(edges.begin(),edges.end());
    size_t i_edge = 0;
#ifndef NDEBUG
    fprintf(stderr,"Checking Edges for Duplicates\n");
#endif
    for (size_t i = 0; i < edges.size(); ++i) {
      Edge& edge = edges[i];
      std::vector <size_t> elements;
      while (i+1 < edges.size() && edge == edges[i+1]) {
        Edge& edge1 = edges[i+1];
        edge.elements.push_back(edge1.elements[0]);
        ++i;
      }
      edges[i_edge] = edge;
      i_edge++;
    }
    edges.resize(i_edge);

#ifndef NDEBUG
    fprintf(stderr,"Setting Edge Properties\n");
#endif
    for (Edge& edge : edges) {
      const Point& p1 = grid.points[edge.p1];
      const Point& p2 = grid.points[edge.p2];
      edge.min.x = std::min(p1.x,p2.x);
      edge.min.y = std::min(p1.y,p2.y);
      edge.min.z = std::min(p1.z,p2.z);
      edge.max.x = std::max(p1.x,p2.x);
      edge.max.y = std::max(p1.y,p2.y);
      edge.max.z = std::max(p1.z,p2.z);
    }
    return edges;
  };

  std::vector<Face> get_faces(const Grid& grid) {
#ifndef NDEBUG
    fprintf(stderr,"Creating Faces\n");
#endif
    std::vector<Face> faces;
    size_t n_faces = 0;
    for (const Element& e : grid.elements) {
      size_t n = Shape::Info[e.type].n_faces;
      if (n == 0) fatal();
      n_faces += n;
    }
    faces.reserve(n_faces);
    //Create Faces from Elements
    for (size_t _e = 0; _e < grid.elements.size(); ++_e) {
      const Element& e = grid.elements[_e];
      if (e.type == Shape::Wedge) {
        Face face1;
        face1.elements.push_back(_e);
        face1.points.push_back(e.points[0]);
        face1.points.push_back(e.points[1]);
        face1.points.push_back(e.points[2]);
        faces.push_back(face1);

        Face face2;
        face2.elements.push_back(_e);
        face2.points.push_back(e.points[3]);
        face2.points.push_back(e.points[4]);
        face2.points.push_back(e.points[5]);
        faces.push_back(face2);

        Face face3a;
        face3a.elements.push_back(_e);
        face3a.points.push_back(e.points[0]);
        face3a.points.push_back(e.points[1]);
        face3a.points.push_back(e.points[4]);
        face3a.points.push_back(e.points[3]);
        faces.push_back(face3a);

        Face face3b;
        face3b.elements.push_back(_e);
        face3b.points.push_back(e.points[1]);
        face3b.points.push_back(e.points[2]);
        face3b.points.push_back(e.points[5]);
        face3b.points.push_back(e.points[4]);
        faces.push_back(face3b);

        Face face3c;
        face3c.elements.push_back(_e);
        face3c.points.push_back(e.points[2]);
        face3c.points.push_back(e.points[0]);
        face3c.points.push_back(e.points[3]);
        face3c.points.push_back(e.points[5]);
        faces.push_back(face3c);
      } else if (e.type == Shape::Triangle || e.type == Shape::Quad) {
        Face face;
        face.elements.push_back(_e);
        face.points = e.points;
        faces.push_back(face);
      } else
        fatal("Shape not supported");
    }
#ifndef NDEBUG
    fprintf(stderr,"Sorting Faces by Points\n");
#endif
    //Create face_sort to sort faces after sorting face points, but maintain index to original vector
    std::vector< std::pair<Face,size_t> > face_sort (faces.size());
    for (size_t i = 0; i < faces.size(); ++i) {
      Face face = faces[i];
      std::sort(face.points.begin(),face.points.end());
      face_sort[i].first = face;
      face_sort[i].second = i;
    }
    std::sort(face_sort.begin(),face_sort.end());
    std::vector <size_t> face_indices;
    size_t i_face = 0;
#ifndef NDEBUG
    fprintf(stderr,"Elimating Duplicate Faces\n");
#endif
    for (size_t i = 0; i < faces.size(); ++i) {
      std::pair <Face,size_t>& face_pair = face_sort[i];
      Face& face = faces[face_pair.second];
      assert(face.elements.size() == 1);
      if (face_pair.first.points.size() == 4) {
        while (i+1 < faces.size() && face_pair.first == face_sort[i+1].first) {
          Face& face1 = faces[face_sort[i+1].second];
          face.elements.push_back(face1.elements[0]);
          ++i;
        }
      }
      face_indices.push_back(face_pair.second);
      i_face++;
    }
    std::sort(face_indices.begin(),face_indices.end());
    for (size_t i = 0; i < face_indices.size(); ++i) {
      assert (i <= face_indices[i]);
      faces[i] = faces[face_indices[i]];
    }
    faces.resize(i_face);

#ifndef NDEBUG
    fprintf(stderr,"Setting Face Properties\n");
#endif
    for (Face& face : faces) {
      const Point& p0 = grid.points[face.points[0]];
      const Point& p1 = grid.points[face.points[1]];
      const Point& p2 = grid.points[face.points[2]];
      face.min.x = std::min(std::min(p0.x,p1.x),p2.x);
      face.min.y = std::min(std::min(p0.y,p1.y),p2.y);
      face.min.z = std::min(std::min(p0.z,p1.z),p2.z);
      face.max.x = std::max(std::max(p0.x,p1.x),p2.x);
      face.max.y = std::max(std::max(p0.y,p1.y),p2.y);
      face.max.z = std::max(std::max(p0.z,p1.z),p2.z);
      if (face.points.size() == 3) {
        face.normal = cross(p1-p0, p2-p1);
        face.center = (p0 + p1 + p2)/3;
      } else {
        const Point& p3 = grid.points[face.points[3]];
        face.normal = cross(p2-p0, p3-p1);

        face.center = Point { 0, 0, 0 };
        double total_length = 0;
        for (size_t i = 0; i < 4; ++i) {
          const Point& p0 = grid.points[face.points[i]];
          const Point& p1 = grid.points[face.points[(i+1)%4]];
          const Point& p2 = grid.points[face.points[(i+2)%4]];

          Vector v1 = p1 - p0;
          Vector v2 = p2 - p1;
          double length = cross(v1,v2).length();

          face.center += (p0 + p1 + p2)/3*length;
          total_length += length;
        }
        if (total_length > 0)
          face.center /= total_length;

        face.min.x = std::min(face.min.x,p3.x);
        face.min.y = std::min(face.min.y,p3.y);
        face.min.z = std::min(face.min.z,p3.z);
        face.max.x = std::max(face.max.x,p3.x);
        face.max.y = std::max(face.max.y,p3.y);
        face.max.z = std::max(face.max.z,p3.z);
      }
    }
    return faces;
  };


  struct Octree {
    std::array < std::unique_ptr<Octree>, 8 > children;
    std::array < std::vector<size_t> , 8> points;
    std::array < std::vector<Edge> , 8> edges;
    std::array < std::vector<Face> , 8> faces;
    Point center;
    double radius;
  };

  Vector quadrant_vector(uint8_t i) {
    Vector v {-1,-1,-1};
    if (i & 1) v.x = 1;
    if (i & 2) v.y = 1;
    if (i & 4) v.z = 1;
    return v;
  }

  std::unique_ptr<Octree> build_octree(const Grid& grid, std::vector<size_t> points, const Point& center, const double radius, size_t depth) {
    std::unique_ptr<Octree> tree (new Octree);
    if (depth > max_depth)
      return nullptr;
    tree->center = center;
    tree->radius = radius;

    for (size_t _p : points) {
      const Point& p = grid.points[_p];
      uint8_t loc = 0;
      if (p.x > center.x) loc |= 1;
      if (p.y > center.y) loc |= 2;
      if (p.z > center.z) loc |= 4;
      tree->points[loc].push_back(_p);
    }

    for (uint8_t i = 0; i < 8; ++i) {
      if (tree->points[i].size() > max_bin_size) {
        Point new_center = center + quadrant_vector(i)*(radius/2);
        tree->children[i] = std::move(build_octree(grid, tree->points[i], new_center, radius/2, depth+1));
        if (tree->children[i])
          tree->points[i].clear();
      }
    }

    return tree;
  }

  std::unique_ptr<Octree> create_octree_root(const Grid& grid) {
    Point min { DBL_MAX, DBL_MAX, DBL_MAX };
    Point max { DBL_MIN, DBL_MIN, DBL_MIN };

    for (const Point& p : grid.points) {
      if (p.x < min.x) min.x = p.x;
      if (p.y < min.y) min.y = p.y;
      if (p.z < min.z) min.z = p.z;
      if (p.x > max.x) max.x = p.x;
      if (p.y > max.y) max.y = p.y;
      if (p.z > max.z) max.z = p.z;
    }

    Point center = (min + max)/2;
    Vector corner = max - center;
    double radius = std::max(std::max(corner.x,corner.y),corner.z);

    std::vector<size_t> points (grid.points.size());;
    for (size_t i = 0; i < grid.points.size(); ++i)
      points[i] = i;

    return build_octree(grid,points,center,radius,0);
  }

  void print_octree(const std::unique_ptr<Octree>& tree,size_t indent) {
    for (size_t i = 0; i < 8; ++i) {
      if (tree->children[i] || tree->points[i].size()) {
        for (size_t j = 0; j < indent; ++j) printf(" ");
        printf("Quadrant %zu -> ",i);

        if (tree->children[i]) {
          printf("\n");
          print_octree(tree->children[i],indent+1);
        } else {
          printf("%lu Points\n",tree->points[i].size());
        }
      }
    }
  }

  Grid octree_to_grid(const std::unique_ptr<Octree>& tree) {
    Grid grid (3);
    for (uint8_t i = 0; i < 8; ++i) {
      if (tree->children[i]) {
        grid += octree_to_grid(tree->children[i]);
      } else if (tree->points[i].size()) {
        Point quad_extreme = tree->center + quadrant_vector(i)*tree->radius;
        size_t _p = grid.points.size();
        grid.points.push_back( Point { tree->center.x, tree->center.y, tree->center.z } );
        grid.points.push_back( Point { quad_extreme.x, tree->center.y, tree->center.z } );
        grid.points.push_back( Point { quad_extreme.x, quad_extreme.y, tree->center.z } );
        grid.points.push_back( Point { tree->center.x, quad_extreme.y, tree->center.z } );
        grid.points.push_back( Point { tree->center.x, tree->center.y, quad_extreme.z } );
        grid.points.push_back( Point { quad_extreme.x, tree->center.y, quad_extreme.z } );
        grid.points.push_back( Point { quad_extreme.x, quad_extreme.y, quad_extreme.z } );
        grid.points.push_back( Point { tree->center.x, quad_extreme.y, quad_extreme.z } );
        Element e (Shape::Hexa);
        for (size_t j = 0; j < 8; ++j)
          e.points[j] = _p+j;
        grid.elements.push_back(e);
      }
    }
    return grid;
  }

  const std::bitset <8> x_bitset ("01010101");
  const std::bitset <8> y_bitset ("00110011");
  const std::bitset <8> z_bitset ("00001111");
  size_t put_edge_in_tree(std::unique_ptr<Octree>& tree, const Edge& edge) {
    size_t n_added = 0;
    std::bitset <8> overlap  ("11111111");

    if (edge.min.x > tree->center.x)
      overlap &= ~x_bitset;
    else if (edge.max.x < tree->center.x)
      overlap &= x_bitset;

    if (edge.min.y > tree->center.y)
      overlap &= ~y_bitset;
    else if (edge.max.y < tree->center.y)
      overlap &= y_bitset;

    if (edge.min.z > tree->center.z)
      overlap &= ~z_bitset;
    else if (edge.max.z < tree->center.z)
      overlap &= z_bitset;

    for (size_t i = 0; i < 8; ++i) {
      if (overlap[i]) {
        if (tree->children[i]) {
          n_added += put_edge_in_tree(tree->children[i],edge);
        } else {
          tree->edges[i].push_back(edge);
          n_added++;
        }
      }
    }
    return n_added;
  }

  size_t put_face_in_tree(std::unique_ptr<Octree>& tree, const Face& face) {
    size_t n_added = 0;
    std::bitset <8> overlap  ("11111111");

    if (face.min.x > tree->center.x)
      overlap &= ~x_bitset;
    else if (face.max.x < tree->center.x)
      overlap &= x_bitset;

    if (face.min.y > tree->center.y)
      overlap &= ~y_bitset;
    else if (face.max.y < tree->center.y)
      overlap &= y_bitset;

    if (face.min.z > tree->center.z)
      overlap &= ~z_bitset;
    else if (face.max.z < tree->center.z)
      overlap &= z_bitset;

    for (size_t i = 0; i < 8; ++i) {
      if (overlap[i]) {
        if (tree->children[i]) {
          n_added += put_face_in_tree(tree->children[i],face);
        } else {
          tree->faces[i].push_back(face);
          n_added++;
        }
      }
    }
    return n_added;
  }

  struct IntersectionsBool {
    std::vector<bool> points;
    std::vector<bool> elements;
  };

  void check_intersections(const Grid& grid, const std::vector<Edge>& edges, const std::vector<Face>& faces, IntersectionsBool& intersections) {
    for (size_t i = 0; i < edges.size(); ++i) {
      const Edge& edge = edges[i];
      const Point& ep1 = grid.points[edge.p1];
      const Point& ep2 = grid.points[edge.p2];
      Vector edge_vector = ep2 - ep1;
      for (size_t j = 0; j < faces.size(); ++j) {
        const Face& face = faces[j];

        if (edge.min.x > face.max.x || edge.min.y > face.max.y || edge.min.z > face.max.z) continue;
        if (edge.max.x < face.min.x || edge.max.y < face.min.y || edge.max.z < face.min.z) continue;

        bool same = false;
        for (size_t p : face.points) {
          if (edge.p1 == p || edge.p2 == p)
            same = true;
          if (ep1 == grid.points[p] || ep2 == grid.points[p])
            same = true;
        }
        if (same) continue;

        double denom = dot(edge_vector,face.normal);
        if (denom == 0) continue;
        double scale = dot(ep1 - face.center,face.normal)/denom;

        if (-1 <= scale && scale <= 0) {
          Point proj = ep1 - edge_vector*scale;
          bool intersected = true;
          for (size_t k = 0; k < face.points.size(); ++k) {
            const Point& p0 = grid.points[face.points[k]];
            const Point& p1 = grid.points[face.points[(k+1)%face.points.size()]];
            Vector v1 = p1 - p0;
            Vector v2 = proj - p1;
            Vector n = cross(v1,v2);
            if (dot(n,face.normal) <= 0){
              intersected = false;
              break;
            }
          }
          if (intersected) {
            for (size_t _e : edge.elements)
              intersections.elements[_e] = true;
            for (size_t _e : face.elements)
              intersections.elements[_e] = true;

            intersections.points[edge.p1] = true;
            intersections.points[edge.p2] = true;
            for (size_t _p : face.points)
              intersections.points[_p] = true;
          }
        }
      }
    }
  }

  void _find_with_tree(const Grid& grid, const std::unique_ptr<Octree>& tree, IntersectionsBool& intersections) {
    for (size_t i = 0; i < 8; ++i) {
      if (tree->children[i])
        _find_with_tree(grid, tree->children[i],intersections);
      else
        check_intersections(grid, tree->edges[i], tree->faces[i],intersections);
    }
  }

  struct IntersectionPair {
    size_t p;
    double dist;
    bool intersected;

    IntersectionPair() : dist(DBL_MAX) {};
  };
  typedef std::vector<IntersectionPair> IntersectionPairs;

  void check_intersections(const Grid& grid, const std::vector<Edge>& edges, const std::vector<Face>& faces, IntersectionPairs& intersections) {
    for (size_t i = 0; i < edges.size(); ++i) {
      const Edge& edge = edges[i];
      double min_dist = intersections[edge.p1].dist;
      const Point& ep1 = grid.points[edge.p1];
      const Point& ep2 = grid.points[edge.p2];
      Vector edge_vector = ep2 - ep1;
      for (size_t j = 0; j < faces.size(); ++j) {
        const Face& face = faces[j];

        if (edge.min.x > face.max.x || edge.min.y > face.max.y || edge.min.z > face.max.z) continue;
        if (edge.max.x < face.min.x || edge.max.y < face.min.y || edge.max.z < face.min.z) continue;

        bool same = false;
        for (size_t p : face.points) {
          if (edge.p1 == p || edge.p2 == p)
            same = true;
          if (ep1 == grid.points[p] || ep2 == grid.points[p])
            same = true;
        }
        if (same) continue;

        double denom = dot(edge_vector,face.normal);
        if (denom == 0) continue;
        double scale = dot(ep1 - face.center,face.normal)/denom;

        if (-1 <= scale && scale <= 0) {
          Point proj = ep1 - edge_vector*scale;
          bool intersected = true;
          for (size_t k = 0; k < face.points.size(); ++k) {
            const Point& p0 = grid.points[face.points[k]];
            const Point& p1 = grid.points[face.points[(k+1)%face.points.size()]];
            Vector v1 = p1 - p0;
            Vector v2 = proj - p1;
            Vector n = cross(v1,v2);
            if (dot(n,face.normal) <= 0){
              intersected = false;
              break;
            }
          }
          if (intersected) {
            for (size_t _p : face.points) {
              const Point& fp = grid.points[_p];
              double d = (fp - ep1).length();
              if (d < min_dist) {
                min_dist = d;
                intersections[edge.p1].p = _p;
                intersections[edge.p1].dist = d;
                intersections[edge.p1].intersected = true;
              }
            }
          }
        }
      }
    }
  }

  void _find_with_tree(const Grid& grid, const std::unique_ptr<Octree>& tree, IntersectionPairs& intersections) {
    for (size_t i = 0; i < 8; ++i) {
      if (tree->children[i])
        _find_with_tree(grid, tree->children[i],intersections);
      else
        check_intersections(grid, tree->edges[i], tree->faces[i],intersections);
    }
  }

  Intersections Intersections::find_with_octree(const Grid& grid) {
    std::vector <Face> faces = get_faces(grid);
    std::vector <Edge> edges = get_edges(grid);

    auto tree = create_octree_root(grid);

    size_t n_edges_added = 0;
    for (const Edge& edge : edges)
      n_edges_added += put_edge_in_tree(tree,edge);

    size_t n_faces_added = 0;
    for (const Face& face : faces)
      n_faces_added += put_face_in_tree(tree,face);

    IntersectionsBool intersections_octree;
    intersections_octree.points.resize(grid.points.size(),false);
    intersections_octree.elements.resize(grid.elements.size(),false);
    _find_with_tree(grid, tree, intersections_octree);

    Intersections intersections;
    for (size_t i = 0; i < intersections_octree.points.size(); ++i) {
      if (intersections_octree.points[i])
        intersections.points.push_back(i);
    }
    for (size_t i = 0; i < intersections_octree.elements.size(); ++i) {
      if (intersections_octree.elements[i])
        intersections.elements.push_back(i);
    }
    return intersections;
  }

  Intersections Intersections::find(const Grid& grid) {
    std::vector <Face> faces = get_faces(grid);
    std::sort(faces.begin(),faces.end(),Face::compare_by_min_x);

    std::vector <Edge> edges = get_edges(grid);
    std::sort(edges.begin(),edges.end(),Edge::compare_by_min_x);

    size_t j_current = 0;
    Intersections intersections;
    std::vector <bool> intersected_points (grid.points.size(),false);
    std::vector <bool> intersected_elements (grid.elements.size(),false);
    for (size_t i = 0; i < edges.size(); ++i) {
      const Edge& edge = edges[i];
      const Point& ep1 = grid.points[edge.p1];
      const Point& ep2 = grid.points[edge.p2];
      Vector edge_vector = ep2 - ep1;
      for (size_t j = j_current; j < faces.size(); ++j) {
        const Face& face = faces[j];
        if (edge.min.x > face.max.x)
          j_current++;
        else
          break;
      }
      for (size_t j = j_current; j < faces.size(); ++j) {
        const Face& face = faces[j];
        if (face.min.x > edge.max.x) break;
        if (edge.min.x > face.max.x || edge.min.y > face.max.y || edge.min.z > face.max.z) continue;
        if (edge.max.x < face.min.x || edge.max.y < face.min.y || edge.max.z < face.min.z) continue;
        bool same = false;
        for (size_t p : face.points) {
          if (edge.p1 == p || edge.p2 == p)
            same = true;
          if (ep1 == grid.points[p] || ep2 == grid.points[p])
            same = true;
        }
        if (same) continue;
        double denom = dot(edge_vector,face.normal);
        if (denom == 0) continue;
        double scale = dot(ep1 - face.center,face.normal)/denom;
        if (-1 <= scale && scale <= 0) {
          Point proj = ep1 - edge_vector*scale;
          bool intersected = true;
          for (size_t k = 0; k < face.points.size(); ++k) {
            const Point& p0 = grid.points[face.points[k]];
            const Point& p1 = grid.points[face.points[(k+1)%face.points.size()]];
            Vector v1 = p1 - p0;
            Vector v2 = proj - p1;
            Vector n = cross(v1,v2);
            if (dot(n,face.normal) <= 0){
              intersected = false;
              break;
            }
          }
          if (intersected) {
            for (size_t _e : edge.elements)
              intersected_elements[_e] = true;
            for (size_t _e : face.elements)
              intersected_elements[_e] = true;

            intersected_points[edge.p1] = true;
            intersected_points[edge.p2] = true;
            for (size_t _p : face.points)
              intersected_points[_p] = true;
          }
        }
      }
    }
    for (size_t i = 0; i < intersected_points.size(); ++i) {
      if (intersected_points[i])
        intersections.points.push_back(i);
    }
    for (size_t i = 0; i < intersected_elements.size(); ++i) {
      if (intersected_elements[i])
        intersections.elements.push_back(i);
    }
    return intersections;
  }

  /*
    This is based on the formula used to determine scaling factor on based on distance
    to next surface [currently x^2/(x^2 + 50)]. The cutoff value is the point at which we
    scaling factor that we stop caring about intersections. We solve for the factor applied
    to the offset edges to check for intersections
  */
  const double future_cutoff = 0.9;
  const double future_factor = sqrt(10*future_cutoff/(1 - future_cutoff));

  double Intersections::get_scale_factor(double distance) {
    return distance*distance/(distance*distance + 50);
  }

  PointPairList Intersections::find_future(const Grid& surface, Grid offset) {

    std::vector <Face> faces = get_faces(surface);

    auto tree = create_octree_root(surface);
    for (const Face& face : faces)
      put_face_in_tree(tree,face);

    size_t n_points = surface.points.size();

    Grid grid (surface);
    grid.points.resize(n_points*2);
    for (size_t _p = 0; _p < n_points; _p++) {
      const Point& surface_p = surface.points[_p];
      const Point& offset_p = offset.points[_p];

      Vector n = offset_p - surface_p;
      Point future_p = surface_p + future_factor*n;
      grid.points[_p + n_points] = future_p;

      Edge edge (_p, _p + n_points);
      edge.min.x = std::min(surface_p.x, future_p.x);
      edge.min.y = std::min(surface_p.y, future_p.y);
      edge.min.z = std::min(surface_p.z, future_p.z);
      edge.max.x = std::max(surface_p.x, future_p.x);
      edge.max.y = std::max(surface_p.y, future_p.y);
      edge.max.z = std::max(surface_p.z, future_p.z);

      put_edge_in_tree(tree,edge);
    }

    IntersectionPairs intersections (n_points);
    _find_with_tree(grid, tree, intersections);
    size_t n_intersections = 0;
    for (auto intersection : intersections)
      if (intersection.intersected)
        n_intersections++;
    PointPairList intersection_list;
    intersection_list.reserve(n_intersections);
    for (size_t _p = 0; _p < n_points; ++_p) {
      if (intersections[_p].intersected) {
        size_t _p2 = intersections[_p].p;
        intersection_list.push_back( std::make_pair (_p,_p2) );
      }
    }
    return intersection_list;
  }

} // namespace unstruc
