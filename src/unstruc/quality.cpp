
#include "quality.h"

#include <utility>

#include "grid.h"
#include "io.h"
#include "point.h"
#include "error.h"

namespace unstruc {

MinMax get_minmax_face_angle(const Grid& grid, const Element& e) {
	MinMax minmax { 360, 0 };
	switch (e.type) {
		case Shape::Triangle:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];

				minmax.update( angle_between(p1 - p0, p2 - p0) );
				minmax.update( angle_between(p2 - p1, p0 - p1) );
				minmax.update( angle_between(p0 - p2, p1 - p2) );
			}
			break;
		case Shape::Quad:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];

				minmax.update( angle_between(p1 - p0, p3 - p0) );
				minmax.update( angle_between(p2 - p1, p0 - p1) );
				minmax.update( angle_between(p3 - p2, p1 - p2) );
				minmax.update( angle_between(p0 - p3, p2 - p3) );
			}
			break;
		case Shape::Tetra:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];

				minmax.update( angle_between(p1 - p0, p2 - p0) );
				minmax.update( angle_between(p2 - p1, p0 - p1) );
				minmax.update( angle_between(p0 - p2, p1 - p2) );

				minmax.update( angle_between(p1 - p0, p3 - p0) );
				minmax.update( angle_between(p3 - p1, p0 - p1) );
				minmax.update( angle_between(p0 - p3, p1 - p3) );

				minmax.update( angle_between(p2 - p1, p3 - p1) );
				minmax.update( angle_between(p3 - p2, p1 - p2) );
				minmax.update( angle_between(p1 - p3, p2 - p3) );

				minmax.update( angle_between(p0 - p2, p3 - p2) );
				minmax.update( angle_between(p3 - p0, p2 - p0) );
				minmax.update( angle_between(p2 - p3, p0 - p3) );
			}
			break;
		case Shape::Wedge:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];
				const Point& p4 = grid.points[e.points[4]];
				const Point& p5 = grid.points[e.points[5]];

				minmax.update( angle_between(p1 - p0, p2 - p0) );
				minmax.update( angle_between(p2 - p1, p0 - p1) );
				minmax.update( angle_between(p0 - p2, p1 - p2) );

				minmax.update( angle_between(p4 - p3, p5 - p3) );
				minmax.update( angle_between(p5 - p4, p3 - p4) );
				minmax.update( angle_between(p3 - p5, p4 - p5) );

				minmax.update( angle_between(p1 - p0, p3 - p0) );
				minmax.update( angle_between(p4 - p1, p0 - p1) );
				minmax.update( angle_between(p3 - p4, p1 - p4) );
				minmax.update( angle_between(p0 - p3, p4 - p3) );

				minmax.update( angle_between(p2 - p1, p4 - p1) );
				minmax.update( angle_between(p5 - p2, p1 - p2) );
				minmax.update( angle_between(p4 - p5, p2 - p5) );
				minmax.update( angle_between(p1 - p4, p5 - p4) );

				minmax.update( angle_between(p0 - p2, p5 - p2) );
				minmax.update( angle_between(p3 - p0, p2 - p0) );
				minmax.update( angle_between(p5 - p3, p0 - p3) );
				minmax.update( angle_between(p2 - p5, p3 - p5) );
			}
			break;
		case Shape::Pyramid:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];
				const Point& p4 = grid.points[e.points[4]];

				minmax.update( angle_between(p1 - p0, p3 - p0) );
				minmax.update( angle_between(p2 - p1, p0 - p1) );
				minmax.update( angle_between(p3 - p2, p1 - p2) );
				minmax.update( angle_between(p0 - p3, p2 - p3) );

				minmax.update( angle_between(p1 - p0, p4 - p0) );
				minmax.update( angle_between(p4 - p1, p0 - p1) );
				minmax.update( angle_between(p0 - p4, p1 - p4) );

				minmax.update( angle_between(p2 - p1, p4 - p1) );
				minmax.update( angle_between(p4 - p2, p1 - p2) );
				minmax.update( angle_between(p1 - p4, p2 - p4) );

				minmax.update( angle_between(p3 - p2, p4 - p2) );
				minmax.update( angle_between(p4 - p3, p2 - p3) );
				minmax.update( angle_between(p2 - p4, p3 - p4) );

				minmax.update( angle_between(p0 - p3, p4 - p3) );
				minmax.update( angle_between(p4 - p0, p3 - p0) );
				minmax.update( angle_between(p3 - p4, p0 - p4) );
			}
			break;
		default:
			not_implemented("(unstruc::get_minmax_angle) Unsupported element type: "+Shape::Info[e.type].name);
	}
	return minmax;
}

MinMax get_minmax_dihedral_angle(const Grid& grid, const Element& e) {
	MinMax minmax { 360, 0 };
	switch (e.type) {
		case Shape::Triangle:
		case Shape::Quad:
			break;
		case Shape::Tetra:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];

				Vector n0 = cross(p1 - p3, p3 - p2);
				Vector n1 = cross(p2 - p3, p3 - p0);
				Vector n2 = cross(p0 - p3, p3 - p1);
				Vector n3 = cross(p1 - p0, p2 - p1);

				minmax.update( angle_between(n0,n1) );
				minmax.update( angle_between(n0,n2) );
				minmax.update( angle_between(n0,n3) );
				minmax.update( angle_between(n1,n2) );
				minmax.update( angle_between(n1,n3) );
				minmax.update( angle_between(n2,n3) );
			}
			break;
		case Shape::Wedge:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];
				const Point& p4 = grid.points[e.points[4]];
				const Point& p5 = grid.points[e.points[5]];

				Vector t0 = cross(p1 - p2, p0 - p1);
				Vector t1 = cross(p4 - p3, p5 - p4);
				Vector q1 = cross(p4 - p0, p3 - p1);
				Vector q2 = cross(p5 - p1, p4 - p2);
				Vector q3 = cross(p3 - p2, p5 - p0);

				minmax.update( angle_between(t0,q1) );
				minmax.update( angle_between(t0,q2) );
				minmax.update( angle_between(t0,q3) );

				minmax.update( angle_between(t1,q1) );
				minmax.update( angle_between(t1,q2) );
				minmax.update( angle_between(t1,q3) );

				minmax.update( angle_between(q1,q2) );
				minmax.update( angle_between(q2,q3) );
				minmax.update( angle_between(q3,q1) );
			}
			break;
		case Shape::Pyramid:
			{
				const Point& p0 = grid.points[e.points[0]];
				const Point& p1 = grid.points[e.points[1]];
				const Point& p2 = grid.points[e.points[2]];
				const Point& p3 = grid.points[e.points[3]];
				const Point& p4 = grid.points[e.points[4]];

				Vector q0 = cross(p2 - p0, p3 - p1);
				Vector t1 = cross(p4 - p0, p1 - p4);
				Vector t2 = cross(p4 - p1, p2 - p4);
				Vector t3 = cross(p4 - p2, p3 - p4);
				Vector t4 = cross(p4 - p3, p0 - p4);

				minmax.update( angle_between(q0,t1) );
				minmax.update( angle_between(q0,t2) );
				minmax.update( angle_between(q0,t3) );
				minmax.update( angle_between(q0,t4) );

				minmax.update( angle_between(t1,t2) );
				minmax.update( angle_between(t2,t3) );
				minmax.update( angle_between(t3,t4) );
				minmax.update( angle_between(t4,t1) );
			}
			break;
		default:
			not_implemented("(unstruc::get_minmax_dihedral_angle) Unsupported element type: "+Shape::Info[e.type].name);
	}
	return minmax;
}

MeshQuality get_mesh_quality(const Grid& grid) {
	MeshQuality quality;
	quality.face_angle.min = 360;
	quality.face_angle.max = 0;
	quality.dihedral_angle.min = 360;
	quality.dihedral_angle.max = 0;
	Grid bad_elements (3);
	bad_elements.points = grid.points;
	for (const Element& e : grid.elements) {
		MinMax f = get_minmax_face_angle(grid,e);
		quality.face_angle.update(f);

		MinMax d = get_minmax_dihedral_angle(grid,e);
		if (d.min < 1.0 || d.max > 179.0)
			bad_elements.elements.push_back(e);
		quality.dihedral_angle.update(d);
	}
	if (bad_elements.elements.size())
		write_grid("bad_elements.vtk",bad_elements);
	return quality;
}

} //namespace
