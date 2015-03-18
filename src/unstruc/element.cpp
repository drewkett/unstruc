#include "element.h"

#include "point.h"
#include "error.h"
#include "grid.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace unstruc {

const Shape Shape::Info[Shape::NShapes] {
	{ "Undefined", 0, 0, 0,  0, 0  },
	{ "Line"     , 1, 2, 1,  0, 3  },
	{ "Triangle" , 2, 3, 3,  1, 5  },
	{ "Quad"     , 2, 4, 4,  1, 9  },
	{ "Polygon"  , 2, 0, 0,  1, 7  },
	{ "Tetra"    , 3, 4, 6,  4, 10 },
	{ "Hexa"     , 3, 8, 12, 6, 12 },
	{ "Wedge"    , 3, 6, 9,  5, 13 },
	{ "Pyramid"  , 3, 5, 8,  5, 14 }
};

Shape::Type type_from_vtk_id(int vtk_id) {
	for (int i = 0; i < Shape::NShapes; ++i) {
		if (Shape::Info[i].vtk_id == vtk_id)
			return static_cast<Shape::Type>(i);
	}
	return Shape::Undefined;
}

void WrongElement(Shape::Type T1, Shape::Type T2) {
	std::ostringstream oss;
	oss << "Wrong Element Type" << std::endl;
	oss << "Got: " << Shape::Info[T1].name << " Expected: " << Shape::Info[T2].name;
	fatal(oss.str());
}

Element::Element(Shape::Type T) : type(T) {
	name_i = 0;
	points.resize(Shape::Info[T].n_points);
}

void dump(const Element &e) {
	std::cerr << "Element " << Shape::Info[e.type].name << std::endl;
	for (int p : e.points) {
		printf("Point %d\n",p);
	}
};

void dump(const Element &e, const Grid &grid) {
	std::cerr << "Element " << Shape::Info[e.type].name << std::endl;
	for (int p : e.points) {
		printf("Point %d : ",p);
		dump(grid.points[p]);
	}
};

double Element::calc_volume(const Grid& grid) const {
	switch (type) {
		case Shape::Line:
		case Shape::Triangle:
		case Shape::Quad:
		case Shape::Polygon:
			return 0;
		case Shape::Hexa:
			{
				const Point& p0 = grid.points[points[0]];
				const Point& p1 = grid.points[points[1]];
				const Point& p2 = grid.points[points[2]];
				const Point& p3 = grid.points[points[3]];
				const Point& p4 = grid.points[points[4]];
				const Point& p5 = grid.points[points[5]];
				const Point& p6 = grid.points[points[6]];
				const Point& p7 = grid.points[points[7]];

				Point face_center1 { 0, 0, 0};
				double total_length1 = 0;
				for (int i = 0; i < 4; ++i) {
					int j = (i + 1)%4;
					const Point& pi = grid.points[points[i]];
					const Point& pj = grid.points[points[j]];
					double l = (pj - pi).length();
					total_length1 += l;

					face_center1.x += l*(pi.x + pj.x)/2;
					face_center1.y += l*(pi.y + pj.y)/2;
					face_center1.z += l*(pi.z + pj.z)/2;
				}
				face_center1.x /= total_length1;
				face_center1.y /= total_length1;
				face_center1.z /= total_length1;

				Point face_center2 { 0, 0, 0 };
				double total_length2 = 0;
				for (int i = 4; i < 8; ++i) {
					int j = 4 + (i + 1)%4;
					const Point& pi = grid.points[points[i]];
					const Point& pj = grid.points[points[j]];
					double l = (pj - pi).length();
					total_length2 += l;

					face_center2.x += l*(pi.x + pj.x)/2;
					face_center2.y += l*(pi.y + pj.y)/2;
					face_center2.z += l*(pi.z + pj.z)/2;
				}
				face_center2.x /= total_length2;
				face_center2.y /= total_length2;
				face_center2.z /= total_length2;

				Vector l = face_center2 - face_center1;
				Vector v02 = p2 - p0;
				Vector v13 = p3 - p1;
				Vector n1 = cross(v02,v13)/2;

				Vector v46 = p6 - p4;
				Vector v57 = p7 - p5;
				Vector n2 = cross(v46,v57)/2;

				return (dot(l,n1) + dot(l,n2))/2;
			}
		case Shape::Tetra:
			{
				const Point& p0 = grid.points[points[0]];
				const Point& p1 = grid.points[points[1]];
				const Point& p2 = grid.points[points[2]];
				const Point& p3 = grid.points[points[3]];
				Vector v1 = p1 - p0;
				Vector v2 = p2 - p1;
				Vector v = cross(v1,v2);
				Vector v3 = p3 - p1;
				return dot(v,v3)/6;
			}
		case Shape::Wedge:
			{
				const Point& p0 = grid.points[points[0]];
				const Point& p1 = grid.points[points[1]];
				const Point& p2 = grid.points[points[2]];
				const Point& p3 = grid.points[points[3]];
				const Point& p4 = grid.points[points[4]];
				const Point& p5 = grid.points[points[5]];

				Point c1;
				c1.x = (p0.x + p1.x + p2.x)/3;
				c1.y = (p0.y + p1.y + p2.y)/3;
				c1.z = (p0.z + p1.z + p2.z)/3;

				Point c2;
				c2.x = (p3.x + p4.x + p5.x)/3;
				c2.y = (p3.y + p4.y + p5.y)/3;
				c2.z = (p3.z + p4.z + p5.z)/3;

				Vector l = c1 - c2;
				Vector v01 = p1 - p0;
				Vector v12 = p2 - p1;
				Vector n1 = cross(v01,v12)/2;
				Vector v34 = p4 - p3;
				Vector v45 = p5 - p4;
				Vector n2 = cross(v34,v45)/2;
				return (dot(l,n1) + dot(l,n2))/2;
			}
		case Shape::Pyramid:
			{
				const Point& p0 = grid.points[points[0]];
				const Point& p1 = grid.points[points[1]];
				const Point& p2 = grid.points[points[2]];
				const Point& p3 = grid.points[points[3]];
				const Point& p4 = grid.points[points[4]];

				Vector v1 = p2 - p0;
				Vector v2 = p3 - p1;
				Vector v = cross(v1,v2);
				Vector v3 = p4 - p0;
				Vector v4 = p4 - p1;
				return dot(v3,v)/12 + dot(v4,v)/12;
			}
		default:
			fatal("Invalid Element Type");
	}
	return 0;
}

bool same(Element e1, Element e2) {
	if (e1.type != e2.type) return false;
	std::sort(e1.points.begin(),e1.points.end());
	std::sort(e2.points.begin(),e2.points.end());
	for (int i = 0; i < e1.points.size(); i++) {
		if (e1.points[i] != e2.points[i])
			return false;
	}
	return true;
};

bool can_collapse(const Element& e) {
	for (int i = 0; i < e.points.size()-1; i++)
		for (int j = i+1; j < e.points.size(); j++)
			if (e.points[i] == e.points[j]) return true;
	return false;
}

bool collapse_tri(Element& e) {
	if (e.type != Shape::Triangle) WrongElement(e.type,Shape::Triangle);
	return can_collapse(e);
}

void tri_from_quad(Element& e,int i1, int i2, int i3) {
	if (e.type != Shape::Quad) WrongElement(e.type,Shape::Quad);
	Element e_new = Element(Shape::Triangle);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.name_i = e.name_i;
	e = e_new;
}

bool collapse_quad(Element& e, std::vector<Element>& new_elements) {
	if (e.type != Shape::Quad) WrongElement(e.type,Shape::Quad);
	if (can_collapse(e)) {
		if (e.points[0] == e.points[1]) {
			tri_from_quad(e,0,2,3);
		} else if (e.points[1] == e.points[2]) {
			tri_from_quad(e,0,1,3);
		} else if (e.points[2] == e.points[3]) {
			tri_from_quad(e,0,1,2);
		} else if (e.points[3] == e.points[0]) {
			tri_from_quad(e,0,1,2);
		} else {
			fatal("Don't know how to collapse quad into tri");
		}
		return collapse_tri(e);
	} else {
		return false;
	}
}

void tetra_from_pyramid(Element& e,int i1, int i2, int i3, int i4) {
	if (e.type != Shape::Pyramid) WrongElement(e.type,Shape::Pyramid);
	Element e_new = Element(Shape::Tetra);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.name_i = e.name_i;
	e = e_new;
}

void pyramid_from_wedge(Element &e,int i1, int i2, int i3, int i4, int i5) {
	if (e.type != Shape::Wedge) WrongElement(e.type,Shape::Wedge);
	Element e_new = Element(Shape::Pyramid);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.name_i = e.name_i;
	e = e_new;
}

void pyramid_from_hexa(Element& e,int i1, int i2, int i3, int i4, int i5) {
	if (e.type != Shape::Hexa) WrongElement(e.type,Shape::Hexa);
	Element e_new = Element(Shape::Pyramid);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.name_i = e.name_i;
	e = e_new;
}

void wedge_from_hexa(Element& e,int i1, int i2, int i3, int i4, int i5, int i6) {
	if (e.type != Shape::Hexa) WrongElement(e.type,Shape::Hexa);
	Element e_new = Element(Shape::Wedge);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.points[5] = e.points[i6];
	e_new.name_i = e.name_i;
	e = e_new;
}

bool collapse_tetra(Element& e) {
	if (e.type != Shape::Tetra) WrongElement(e.type,Shape::Tetra);
	return can_collapse(e);
}

bool collapse_pyramid(Element& e) {
	if (e.type != Shape::Pyramid) WrongElement(e.type,Shape::Pyramid);
	if (can_collapse(e)) {
		if (e.points[0] == e.points[1]) {
			tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e);
		} else if (e.points[1] == e.points[2]) {
			tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e);
		} else if (e.points[2] == e.points[3]) {
			tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e);
		} else if (e.points[3] == e.points[0]) {
			tetra_from_pyramid(e,0,1,2,4);
			return collapse_tetra(e);
		} else if (e.points[0] == e.points[4]) {
			tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e);
		} else if (e.points[1] == e.points[4]) {
			tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e);
		} else if (e.points[2] == e.points[4]) {
			tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e);
		} else if (e.points[3] == e.points[4]) {
			tetra_from_pyramid(e,0,1,2,4);
			return collapse_tetra(e);
		} else {
			not_implemented("Don't know how to collapse this pyramid");
		}
	}
	return false;
}

bool collapse_wedge(Element& e, std::vector<Element>& new_elements) {
	if (e.type != Shape::Wedge) WrongElement(e.type,Shape::Wedge);
	if (can_collapse(e)) {
		if (e.points[0] == e.points[3]) {
			pyramid_from_wedge(e,1,2,5,4,0);
			return collapse_pyramid(e);
		} else if (e.points[1] == e.points[4]) {
			pyramid_from_wedge(e,2,0,3,5,1);
			return collapse_pyramid(e);
		} else if (e.points[2] == e.points[5]) {
			pyramid_from_wedge(e,0,1,4,3,2);
			return collapse_pyramid(e);
		} else {
			not_implemented("Don't know how to collapse this wedge");
		}
	}
	return false;
}

bool collapse_hexa(Element &e,std::vector<Element>& new_elements) {
	if (e.type != Shape::Hexa) WrongElement(e.type,Shape::Hexa);
	if (can_collapse(e)) {
        if (e.points[0] == e.points[1]) {
			if (e.points[2] == e.points[3]) {
				wedge_from_hexa(e,0,4,5,3,7,6);
				return collapse_wedge(e,new_elements);
			} else if (e.points[4] == e.points[5]) {
				wedge_from_hexa(e,1,2,3,5,6,7);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,2,5,6,3,4,7);
				pyramid_from_hexa(e_pyramid,2,5,4,3,0);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[1] == e.points[2]) {
			if (e.points[0] == e.points[3]) {
				wedge_from_hexa(e,1,5,6,0,4,7);
				return collapse_wedge(e,new_elements);
			} else if (e.points[5] == e.points[6]) {
				wedge_from_hexa(e,2,3,0,6,7,4);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,3,6,7,0,5,4);
				pyramid_from_hexa(e_pyramid,3,6,5,0,1);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[2] == e.points[3]) {
			if (e.points[6] == e.points[7]) {
				wedge_from_hexa(e,3,0,1,7,4,5);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,0,7,4,1,6,5);
				pyramid_from_hexa(e_pyramid,0,7,6,1,2);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[3] == e.points[0]) {
			if (e.points[7] == e.points[4]) {
				wedge_from_hexa(e,0,1,2,4,5,6);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,1,4,5,2,7,6);
				pyramid_from_hexa(e_pyramid,1,4,7,2,3);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[0] == e.points[4]) {
			if (e.points[1] == e.points[5]) {
				wedge_from_hexa(e,0,3,7,1,2,6);
				return collapse_wedge(e,new_elements);
			} else if (e.points[3] == e.points[7]) {
				wedge_from_hexa(e,3,2,6,0,1,5);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,1,2,3,5,6,7);
				pyramid_from_hexa(e_pyramid,3,1,5,7,0);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[1] == e.points[5]) {
			if (e.points[2] == e.points[6]) {
				wedge_from_hexa(e,1,0,4,2,3,7);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,2,3,0,6,7,4);
				pyramid_from_hexa(e_pyramid,0,2,6,4,1);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[2] == e.points[6]) {
			if (e.points[3] == e.points[7]) {
				wedge_from_hexa(e,2,1,5,3,0,4);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,3,0,1,7,4,5);
				pyramid_from_hexa(e_pyramid,1,3,7,5,2);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[3] == e.points[7]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,0,1,2,4,5,6);
			pyramid_from_hexa(e_pyramid,2,0,4,6,3);
			if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
			new_elements.push_back(e_pyramid);
			return collapse_wedge(e,new_elements);
		} else if (e.points[4] == e.points[5]) {
			if (e.points[6] == e.points[7]) {
				wedge_from_hexa(e,1,0,4,2,3,7);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,0,3,7,1,2,6);
				pyramid_from_hexa(e_pyramid,0,1,6,7,4);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[5] == e.points[6]) {
			if (e.points[7] == e.points[4]) {
				wedge_from_hexa(e,2,1,5,3,0,4);
				return collapse_wedge(e,new_elements);
			} else {
				Element e_pyramid = e;
				wedge_from_hexa(e,1,0,4,2,3,7);
				pyramid_from_hexa(e_pyramid,1,2,7,4,5);
				if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[6] == e.points[7]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,2,1,5,3,0,4);
			pyramid_from_hexa(e_pyramid,2,3,4,5,6);
			if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
			new_elements.push_back(e_pyramid);
			return collapse_wedge(e,new_elements);
		} else if (e.points[7] == e.points[4]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,3,2,6,0,1,5);
			pyramid_from_hexa(e_pyramid,3,0,5,6,7);
			if (collapse_pyramid(e_pyramid)) not_implemented("Return from Pyramid Collapse");
			new_elements.push_back(e_pyramid);
			return collapse_wedge(e,new_elements);
		}
	}
	return false;
}

bool collapse(Element &e,std::vector<Element>& new_elements) {
	switch (e.type) {
		case Shape::Quad:
			return collapse_quad(e,new_elements);
		case Shape::Hexa:
			return collapse_hexa(e,new_elements);
		default:
			fprintf(stderr,"Eltype == %s\n",Shape::Info[e.type].name.c_str());
			not_implemented("Collapse for ElType");
	}
	return false;
};

bool collapse_wedge_wo_split(Element& e) {
	if (e.type != Shape::Wedge) WrongElement(e.type,Shape::Wedge);
	if (can_collapse_wo_split(e)) {
		if (e.points[0] == e.points[3]) {
			pyramid_from_wedge(e,1,2,5,4,0);
			return collapse_pyramid(e);
		} else if (e.points[1] == e.points[4]) {
			pyramid_from_wedge(e,2,0,3,5,1);
			return collapse_pyramid(e);
		} else if (e.points[2] == e.points[5]) {
			pyramid_from_wedge(e,0,1,4,3,2);
			return collapse_pyramid(e);
		} else {
			not_implemented("Don't know how to collapse this wedge");
		}
	}
	return false;
}

bool can_collapse_wo_split(const Element& e) {
	switch (e.type) {
		case Shape::Wedge:
			return (e.points[0] == e.points[3] || e.points[1] == e.points[4] || e.points[2] == e.points[5]);
		case Shape::Triangle:
		case Shape::Quad:
		case Shape::Tetra:
		case Shape::Pyramid:
			return can_collapse(e);
		default:
			fprintf(stderr,"Eltype == %s\n",Shape::Info[e.type].name.c_str());
			not_implemented("can_collapse_wo_split for ElType");
	}
	return false;
}

bool collapse_wo_split(Element &e) {
	switch (e.type) {
		case Shape::Triangle:
			return collapse_tri(e);
		case Shape::Pyramid:
			return collapse_pyramid(e);
		case Shape::Tetra:
			return collapse_tetra(e);
		case Shape::Wedge:
			return collapse_wedge_wo_split(e);
		default:
			fprintf(stderr,"Eltype == %s\n",Shape::Info[e.type].name.c_str());
			not_implemented("collapse_wo_split for ElType");
	}
	return false;
};

} // namespace unstruc
