#include "element.h"

#include "point.h"
#include "error.h"
#include "grid.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

void WrongElement(int T1, int T2) {
	std::ostringstream oss;
	oss << "Wrong Element Type" << std::endl;
	oss << "Got: " << T1 << " Expected: " << T2;
	Fatal(oss.str());
}

Element::Element(int T) : type(T) {
	name_i = 0;
	int len = 0;
	switch (T) {
		case LINE:
			len = 2;
			dim = 1;
			break;
		case TRI:
			len = 3;
			dim = 2;
			break;
		case QUAD:
			len = 4;
			dim = 2;
			break;
		case HEXA:
			len = 8;
			dim = 3;
			break;
		case TETRA:
			len = 4;
			dim = 3;
			break;
		case WEDGE:
			len = 6;
			dim = 3;
			break;
		case PYRAMID:
			len = 5;
			dim = 3;
			break;
		case POLYGON:
			len = 0;
			dim = 2;
			break;
		default:
			std::ostringstream oss;
			oss << "Element Type " << T;
			NotImplemented(oss.str());
	}
	points.resize(len);
}

void dump(Element &e) {
	std::cerr << "Element " << e.type << std::endl;
	for (int p : e.points) {
		printf("Point %d\n",p);
	}
};

void dump(Element &e,Grid &grid) {
	std::cerr << "Element " << e.type << std::endl;
	for (int p : e.points) {
		printf("Point %d : ",p);
		dump(grid.points[p]);
	}
};

double Element::calc_volume(Grid& grid) {
	switch (type) {
		case LINE:
		case TRI:
		case QUAD:
		case POLYGON:
			return 0;
		case HEXA:
			{
				Point& p0 = grid.points[points[0]];
				Point& p1 = grid.points[points[1]];
				Point& p2 = grid.points[points[2]];
				Point& p3 = grid.points[points[3]];
				Point& p4 = grid.points[points[4]];
				Point& p5 = grid.points[points[5]];
				Point& p6 = grid.points[points[6]];
				Point& p7 = grid.points[points[7]];

				Point face_center1;
				double total_length1 = 0;
				for (int i = 0; i < 4; ++i) {
					int j = (i + 1)%4;
					Point& pi = grid.points[points[i]];
					Point& pj = grid.points[points[j]];
					double l = (pj - pi).length();
					total_length1 += l;

					face_center1.x += l*(pi.x + pj.x)/2;
					face_center1.y += l*(pi.y + pj.y)/2;
					face_center1.z += l*(pi.z + pj.z)/2;
				}
				face_center1.x /= total_length1;
				face_center1.y /= total_length1;
				face_center1.z /= total_length1;

				Point face_center2;
				double total_length2 = 0;
				for (int i = 4; i < 8; ++i) {
					int j = 4 + (i + 1)%4;
					Point& pi = grid.points[points[i]];
					Point& pj = grid.points[points[j]];
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
		case TETRA:
			{
				Point& p0 = grid.points[points[0]];
				Point& p1 = grid.points[points[1]];
				Point& p2 = grid.points[points[2]];
				Point& p3 = grid.points[points[3]];
				Vector v1 = p1 - p0;
				Vector v2 = p2 - p1;
				Vector v = cross(v1,v2);
				Vector v3 = p3 - p1;
				return dot(v,v3)/6;
			}
		case WEDGE:
			{
				Point& p0 = grid.points[points[0]];
				Point& p1 = grid.points[points[1]];
				Point& p2 = grid.points[points[2]];
				Point& p3 = grid.points[points[3]];
				Point& p4 = grid.points[points[4]];
				Point& p5 = grid.points[points[5]];

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
		case PYRAMID:
			{
				Point& p0 = grid.points[points[0]];
				Point& p1 = grid.points[points[1]];
				Point& p2 = grid.points[points[2]];
				Point& p3 = grid.points[points[3]];
				Point& p4 = grid.points[points[4]];

				Vector v1 = p2 - p0;
				Vector v2 = p3 - p1;
				Vector v = cross(v1,v2);
				Vector v3 = p4 - p0;
				Vector v4 = p4 - p1;
				return dot(v3,v)/12 + dot(v4,v)/12;
			}
		default:
			Fatal("Invalid Element Type");
	}
	return 0;
}

bool same(Element &e1, Element &e2) {
	if (e1.type != e2.type) return false;
	std::vector<int> e1_points (e1.points);
	std::vector<int> e2_points (e2.points);
	std::sort(e1_points.begin(),e1_points.end());
	std::sort(e2_points.begin(),e2_points.end());
	for (int i = 0; i < e1_points.size(); i++) {
		if (e1_points[i] != e2_points[i])
			return false;
	}
	return true;
};

bool can_collapse(Element& e) {
	for (int i = 0; i < e.points.size()-1; i++)
		for (int j = i+1; j < e.points.size(); j++)
			if (e.points[i] == e.points[j]) return true;
	return false;
}

bool collapse_tri(Element& e,std::vector<Element>& new_elements) {
	if (e.type != TRI) WrongElement(e.type,TRI);
	return can_collapse(e);
}

void tri_from_quad(Element& e,int i1, int i2, int i3) {
	if (e.type != QUAD) WrongElement(e.type,QUAD);
	Element e_new = Element(TRI);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.name_i = e.name_i;
	e = e_new;
}

bool collapse_quad(Element& e, std::vector<Element>& new_elements) {
	if (e.type != QUAD) WrongElement(e.type,QUAD);
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
			throw 1;
		}
		return collapse_tri(e,new_elements);
	} else {
		return false;
	}
}

void tetra_from_pyramid(Element& e,int i1, int i2, int i3, int i4) {
	if (e.type != PYRAMID) WrongElement(e.type,PYRAMID);
	Element e_new = Element(TETRA);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.name_i = e.name_i;
	e = e_new;
}

void pyramid_from_wedge(Element &e,int i1, int i2, int i3, int i4, int i5) {
	if (e.type != WEDGE) WrongElement(e.type,WEDGE);
	Element e_new = Element(PYRAMID);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.name_i = e.name_i;
	e = e_new;
}

void pyramid_from_hexa(Element& e,int i1, int i2, int i3, int i4, int i5) {
	if (e.type != HEXA) WrongElement(e.type,HEXA);
	Element e_new = Element(PYRAMID);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.name_i = e.name_i;
	e = e_new;
}

void wedge_from_hexa(Element& e,int i1, int i2, int i3, int i4, int i5, int i6) {
	if (e.type != HEXA) WrongElement(e.type,HEXA);
	Element e_new = Element(WEDGE);
	e_new.points[0] = e.points[i1];
	e_new.points[1] = e.points[i2];
	e_new.points[2] = e.points[i3];
	e_new.points[3] = e.points[i4];
	e_new.points[4] = e.points[i5];
	e_new.points[5] = e.points[i6];
	e_new.name_i = e.name_i;
	e = e_new;
}

bool collapse_tetra(Element& e,std::vector<Element>& new_elements) {
	if (e.type != TETRA) WrongElement(e.type,TETRA);
	return can_collapse(e);
}

bool collapse_pyramid(Element& e,std::vector<Element>& new_elements) {
	if (e.type != PYRAMID) WrongElement(e.type,PYRAMID);
	if (can_collapse(e)) {
		if (e.points[0] == e.points[1]) {
			tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[1] == e.points[2]) {
			tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[2] == e.points[3]) {
			tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[0] == e.points[4]) {
			tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[1] == e.points[4]) {
			tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[2] == e.points[4]) {
			tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e,new_elements);
		} else if (e.points[3] == e.points[4]) {
			tetra_from_pyramid(e,0,1,2,4);
			return collapse_tetra(e,new_elements);
		} else {
			NotImplemented("Don't know how to collapse this pyramid");
		}
	}
	return false;
}

bool collapse_wedge(Element& e, std::vector<Element>& new_elements) {
	if (e.type != WEDGE) WrongElement(e.type,WEDGE);
	if (can_collapse(e)) {
        if (e.points[0] == e.points[1]) {
			pyramid_from_wedge(e,2,1,4,5,3);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[1] == e.points[2]) {
			pyramid_from_wedge(e,0,2,5,3,4);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[2] == e.points[0]) {
			pyramid_from_wedge(e,1,0,3,4,5);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[3] == e.points[4]) {
			pyramid_from_wedge(e,2,1,4,5,0);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[4] == e.points[5]) {
			pyramid_from_wedge(e,0,2,5,3,1);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[5] == e.points[3]) {
			pyramid_from_wedge(e,1,0,3,4,2);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[0] == e.points[3]) {
			pyramid_from_wedge(e,2,1,4,5,0);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[1] == e.points[4]) {
			pyramid_from_wedge(e,0,2,5,3,1);
			return collapse_pyramid(e,new_elements);
		} else if (e.points[2] == e.points[5]) {
			pyramid_from_wedge(e,1,0,3,4,2);
			return collapse_pyramid(e,new_elements);
		} else {
			NotImplemented("Don't know how to collapse this wedge");
		}
	}
	return false;
}

bool collapse_hexa(Element &e,std::vector<Element>& new_elements) {
	if (e.type != HEXA) WrongElement(e.type,HEXA);
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[3] == e.points[7]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,0,1,2,4,5,6);
			pyramid_from_hexa(e_pyramid,2,0,4,6,3);
			if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
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
				if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
				new_elements.push_back(e_pyramid);
				return collapse_wedge(e,new_elements);
			}
		} else if (e.points[6] == e.points[7]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,2,1,5,3,0,4);
			pyramid_from_hexa(e_pyramid,2,3,4,5,6);
			if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
			new_elements.push_back(e_pyramid);
			return collapse_wedge(e,new_elements);
		} else if (e.points[7] == e.points[4]) {
			Element e_pyramid = e;
			wedge_from_hexa(e,3,2,6,0,1,5);
			pyramid_from_hexa(e_pyramid,3,0,5,6,7);
			if (collapse_pyramid(e_pyramid,new_elements)) NotImplemented("Return from Pyramid Collapse");
			new_elements.push_back(e_pyramid);
			return collapse_wedge(e,new_elements);
		}
	}
	return false;
}

bool collapse(Element &e,std::vector<Element>& new_elements) {
	switch (e.type) {
		case QUAD:
			return collapse_quad(e,new_elements);
		case HEXA:
			return collapse_hexa(e,new_elements);
	}
	NotImplemented("Collapse for ElType");
	return false;
};
