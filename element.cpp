#include "element.h"
#include "point.h"
#include "error.h"

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
	name_i = -1;
	i = -1;
	s = NULL;
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
		default:
			std::ostringstream oss;
			oss << "Element Type " << T;
			NotImplemented(oss.str());
	}
	points.resize(len);
	valid = true;
}

void dump(Element &e) {
	std::cerr << "Element " << e.type << std::endl;
	for (int i=0; i < e.len; i++) {
		dump(e.points[i]);
	}
};

double Element::calc_volume() {
	double volume = 0;
	switch (type) {
		case LINE:
		case TRI:
		case QUAD:
			break;
		case HEXA:
			NotImplemented("Hexa volume calculation");
		case TETRA:
			{
				Vector v1 = subtract_points(*points[1],*points[0]);
				Vector v2 = subtract_points(*points[2],*points[1]);
				Vector v = cross(v1,v2);
				Vector v3 = subtract_points(*points[3],*points[1]);
				volume = dot(v,v3)/6;
			}
			break;
		case WEDGE:
			{
				Point c1, c2;
				c1.x = ((*points[0])->x + (*points[1])->x + (*points[2])->x)/3;
				c1.y = ((*points[0])->y + (*points[1])->y + (*points[2])->y)/3;
				c1.z = ((*points[0])->z + (*points[1])->z + (*points[2])->z)/3;
				c2.x = ((*points[3])->x + (*points[4])->x + (*points[5])->x)/3;
				c2.y = ((*points[3])->y + (*points[4])->y + (*points[5])->y)/3;
				c2.z = ((*points[3])->z + (*points[4])->z + (*points[5])->z)/3;
				Vector l = c1 - c2;
				Vector v01 = subtract_points(*points[1],*points[0]);
				Vector v12 = subtract_points(*points[2],*points[1]);
				Vector n1 = cross(v01,v12)/2;
				Vector v34 = subtract_points(*points[4],*points[3]);
				Vector v45 = subtract_points(*points[5],*points[4]);
				Vector n2 = cross(v34,v45)/2;
				volume = (dot(l,n1) + dot(l,n2))/2;
				dump(n1);
				dump(n2);
				dump(l);
				if (volume < 0)
					dump(*this);
			}
		case PYRAMID:
			{
				Vector v1 = subtract_points(*points[2],*points[0]);
				Vector v2 = subtract_points(*points[3],*points[1]);
				Vector v = cross(v1,v2);
				Vector v3 = subtract_points(*points[4],*points[0]);
				Vector v4 = subtract_points(*points[4],*points[1]);
				volume = dot(v3,v)/12 + dot(v4,v)/12;
			}
			break;
		default:
			Fatal("Invalid Element Type");
	}
	return volume;
}

void set_s_by_lowest_id(Element &e) {
	if (!e.valid) return;
	e.s = (*e.points[0]);
	for (int i=1; i < e.len; i++) {
		if ((*e.points[i]) < e.s) {
			e.s = *e.points[i];
		}
	}
}

bool compare_element(Element &e1, Element &e2) {
	if (!e1.valid) return false;
	if (!e2.valid) return true;
	return e1.s < e2.s;
}

bool compare_element_by_name(Element &e1, Element &e2) {
	if (!e1.valid) return false;
	if (!e2.valid) return true;
	return e1.name_i < e2.name_i;
}

bool compare_element_by_index(Element &e1, Element &e2) {
	if (!e1.valid) return false;
	if (!e2.valid) return true;
	return e1.i < e2.i;
}

bool close(Element &e1, Element &e2) {
	return e1.s == e2.s;
};

bool same(Element &e1, Element &e2) {
	if (e1.type != e2.type) return false;
	bool br;
	for (int i = 0; i < e1.len; i++) {
		br = false;
		for (int j = 0; j < e2.len; j++) {
			if (*e1.points[i] == *e2.points[j]) {
				br = true;
				break;
			}
		}
		if (!br) return false;
	}
	for (int j = 0; j < e2.len; j++) {
		br = false;
		for (int i = 0; i < e1.len; i++) {
			if (*e1.points[i] == *e2.points[j]) {
				br = true;
				break;
			}
		}
		if (!br) return false;
	}
	return true;
};

bool canCollapse(Element * e) {
	for (int i = 0; i < e->len-1; i++) {
		for (int j = i+1; j < e->len; j++) {
			if (*e->points[i] == *e->points[j]) return true;
		}
	}
	return false;
}

Element * collapse_tri(Element * e) {
	if (e->type != TRI) WrongElement(e->type,TRI);
	if (canCollapse(e)) e = NULL;
	return NULL;
}

Element * tri_from_quad(Element * e,int i1, int i2, int i3) {
	if (e->type != QUAD) WrongElement(e->type,QUAD);
	Element * e_new = new Element(TRI);
	e_new->points[0] = e->points[i1];
	e_new->points[1] = e->points[i2];
	e_new->points[2] = e->points[i3];
	e_new->name_i = e->name_i;
	e_new->i = e->i;
	return e_new;
}

Element * collapse_quad(Element * e) {
	if (e->type != QUAD) WrongElement(e->type,QUAD);
	if (canCollapse(e)) {
		if (*e->points[0] == *e->points[1]) {
			*e = *tri_from_quad(e,0,2,3);
		} else if (*e->points[1] == *e->points[2]) {
			*e = *tri_from_quad(e,0,1,3);
		} else if (*e->points[2] == *e->points[3]) {
			*e = *tri_from_quad(e,0,1,2);
		} else if (*e->points[3] == *e->points[0]) {
			*e = *tri_from_quad(e,0,1,2);
		} else {
			dump(*e);
			throw 1;
		}
		collapse_tri(e);
	}
	return NULL;
}

Element * tetra_from_pyramid(Element * e,int i1, int i2, int i3, int i4) {
	if (e->type != PYRAMID) WrongElement(e->type,PYRAMID);
	Element * e_new = new Element(TETRA);
	e_new->points[0] = e->points[i1];
	e_new->points[1] = e->points[i2];
	e_new->points[2] = e->points[i3];
	e_new->points[3] = e->points[i4];
	e_new->name_i = e->name_i;
	e_new->i = e->i;
	return e_new;
}

Element * pyramid_from_wedge(Element * e,int i1, int i2, int i3, int i4, int i5) {
	if (e->type != WEDGE) WrongElement(e->type,WEDGE);
	Element * e_new = new Element(PYRAMID);
	e_new->points[0] = e->points[i1];
	e_new->points[1] = e->points[i2];
	e_new->points[2] = e->points[i3];
	e_new->points[3] = e->points[i4];
	e_new->points[4] = e->points[i5];
	e_new->name_i = e->name_i;
	e_new->i = e->i;
	return e_new;
}

Element * pyramid_from_hexa(Element * e,int i1, int i2, int i3, int i4, int i5) {
	if (e->type != HEXA) WrongElement(e->type,HEXA);
	Element * e_new = new Element(PYRAMID);
	e_new->points[0] = e->points[i1];
	e_new->points[1] = e->points[i2];
	e_new->points[2] = e->points[i3];
	e_new->points[3] = e->points[i4];
	e_new->points[4] = e->points[i5];
	e_new->name_i = e->name_i;
	e_new->i = e->i;
	return e_new;
}

Element * wedge_from_hexa(Element * e,int i1, int i2, int i3, int i4, int i5, int i6) {
	if (e->type != HEXA) WrongElement(e->type,HEXA);
	Element * e_new = new Element(WEDGE);
	e_new->points[0] = e->points[i1];
	e_new->points[1] = e->points[i2];
	e_new->points[2] = e->points[i3];
	e_new->points[3] = e->points[i4];
	e_new->points[4] = e->points[i5];
	e_new->points[5] = e->points[i6];
	e_new->name_i = e->name_i;
	e_new->i = e->i;
	return e_new;
}

Element * collapse_tetra(Element * e) {
	if (e->type != TETRA) WrongElement(e->type,TETRA);
	if (canCollapse(e)) e = NULL;
	return NULL;
}

Element * collapse_pyramid(Element * e) {
	if (e->type != PYRAMID) WrongElement(e->type,PYRAMID);
	if (canCollapse(e)) {
		if (*e->points[0] == *e->points[1]) {
			*e = *tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e);
		} else if (*e->points[1] == *e->points[2]) {
			*e = *tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e);
		} else if (*e->points[2] == *e->points[3]) {
			*e = *tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e);
		} else if (*e->points[0] == *e->points[4]) {
			*e = *tetra_from_pyramid(e,1,2,3,4);
			return collapse_tetra(e);
		} else if (*e->points[1] == *e->points[4]) {
			*e = *tetra_from_pyramid(e,2,3,0,4);
			return collapse_tetra(e);
		} else if (*e->points[2] == *e->points[4]) {
			*e = *tetra_from_pyramid(e,3,0,1,4);
			return collapse_tetra(e);
		} else if (*e->points[3] == *e->points[4]) {
			*e = *tetra_from_pyramid(e,0,1,2,4);
			return collapse_tetra(e);
		} else {
			dump(*e);
			NotImplemented("Don't know how to collapse this pyramid");
		}
	}
	return NULL;
}

Element * collapse_wedge(Element * e) {
	if (e->type != WEDGE) WrongElement(e->type,WEDGE);
	if (canCollapse(e)) {
        if (*e->points[0] == *e->points[1]) {
			*e = *pyramid_from_wedge(e,2,1,4,5,3);
			return collapse_pyramid(e);
		} else if (*e->points[1] == *e->points[2]) {
			*e = *pyramid_from_wedge(e,0,2,5,3,4);
			return collapse_pyramid(e);
		} else if (*e->points[2] == *e->points[0]) {
			*e = *pyramid_from_wedge(e,1,0,3,4,5);
			return collapse_pyramid(e);
		} else if (*e->points[3] == *e->points[4]) {
			*e = *pyramid_from_wedge(e,2,1,4,5,0);
			return collapse_pyramid(e);
		} else if (*e->points[4] == *e->points[5]) {
			*e = *pyramid_from_wedge(e,0,2,5,3,1);
			return collapse_pyramid(e);
		} else if (*e->points[5] == *e->points[3]) {
			*e = *pyramid_from_wedge(e,1,0,3,4,2);
			return collapse_pyramid(e);
		} else if (*e->points[0] == *e->points[3]) {
			*e = *pyramid_from_wedge(e,2,1,4,5,0);
			return collapse_pyramid(e);
		} else if (*e->points[1] == *e->points[4]) {
			*e = *pyramid_from_wedge(e,0,2,5,3,1);
			return collapse_pyramid(e);
		} else if (*e->points[2] == *e->points[5]) {
			*e = *pyramid_from_wedge(e,1,0,3,4,2);
			return collapse_pyramid(e);
		} else {
			NotImplemented("Don't know how to collapse this wedge");
		}
	}
	return NULL;
}

Element * collapse_hexa(Element * e) {
	if (e->type != HEXA) WrongElement(e->type,HEXA);
	Element *e_new, *e_new2;
	if (canCollapse(e)) {
        if (*e->points[0] == *e->points[1]) {
			if (*e->points[2] == *e->points[3]) {
				*e = *wedge_from_hexa(e,0,4,5,3,7,6);
				return collapse_wedge(e);
			} else if (*e->points[4] == *e->points[5]) {
				*e = *wedge_from_hexa(e,1,2,3,5,6,7);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,2,5,6,3,4,7);
				e_new2 = pyramid_from_hexa(e,2,5,4,3,0);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[1] == *e->points[2]) {
			if (*e->points[0] == *e->points[3]) {
				*e = *wedge_from_hexa(e,1,5,6,0,4,7);
				return collapse_wedge(e);
			} else if (*e->points[5] == *e->points[6]) {
				*e = *wedge_from_hexa(e,2,3,0,6,7,4);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,3,6,7,0,5,4);
				e_new2 = pyramid_from_hexa(e,3,6,5,0,1);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[2] == *e->points[3]) {
			if (*e->points[6] == *e->points[7]) {
				*e = *wedge_from_hexa(e,3,0,1,7,4,5);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,0,7,4,1,6,5);
				e_new2 = pyramid_from_hexa(e,0,7,6,1,2);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[3] == *e->points[0]) {
			if (*e->points[7] == *e->points[4]) {
				*e = *wedge_from_hexa(e,0,1,2,4,5,6);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,1,4,5,2,7,6);
				e_new2 = pyramid_from_hexa(e,1,4,7,2,3);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[0] == *e->points[4]) {
			if (*e->points[1] == *e->points[5]) {
				*e = *wedge_from_hexa(e,0,3,7,1,2,6);
				return collapse_wedge(e);
			} else if (*e->points[3] == *e->points[7]) {
				*e = *wedge_from_hexa(e,3,2,6,0,1,5);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,1,2,3,5,6,7);
				e_new2 = pyramid_from_hexa(e,3,1,5,7,0);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[1] == *e->points[5]) {
			if (*e->points[2] == *e->points[6]) {
				*e = *wedge_from_hexa(e,1,0,4,2,3,7);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,2,3,0,6,7,4);
				e_new2 = pyramid_from_hexa(e,0,2,6,4,1);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[2] == *e->points[6]) {
			if (*e->points[3] == *e->points[7]) {
				*e = *wedge_from_hexa(e,2,1,5,3,0,4);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,3,0,1,7,4,5);
				e_new2 = pyramid_from_hexa(e,1,3,7,5,2);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[3] == *e->points[7]) {
			e_new = wedge_from_hexa(e,0,1,2,4,5,6);
			e_new2 = pyramid_from_hexa(e,2,0,4,6,3);
			if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
			if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
			*e = *e_new;
			return e_new2;
		} else if (*e->points[4] == *e->points[5]) {
			if (*e->points[6] == *e->points[7]) {
				*e = *wedge_from_hexa(e,1,0,4,2,3,7);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,0,3,7,1,2,6);
				e_new2 = pyramid_from_hexa(e,0,1,6,7,4);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[5] == *e->points[6]) {
			if (*e->points[7] == *e->points[4]) {
				*e = *wedge_from_hexa(e,2,1,5,3,0,4);
				return collapse_wedge(e);
			} else {
				e_new = wedge_from_hexa(e,1,0,4,2,3,7);
				e_new2 = pyramid_from_hexa(e,1,2,7,4,5);
				if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
				if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
				*e = *e_new;
				return e_new2;
			}
		} else if (*e->points[6] == *e->points[7]) {
			e_new = wedge_from_hexa(e,2,1,5,3,0,4);
			e_new2 = pyramid_from_hexa(e,2,3,4,5,6);
			if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
			if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
			*e = *e_new;
			return e_new2;
		} else if (*e->points[7] == *e->points[4]) {
			e_new = wedge_from_hexa(e,3,2,6,0,1,5);
			e_new2 = pyramid_from_hexa(e,3,0,5,6,7);
			if (collapse_wedge(e_new)) NotImplemented("Return from _wedge Collapse 1");
			if (collapse_pyramid(e_new2)) NotImplemented("Return from _wedge Collapse 2");
			*e = *e_new;
			return e_new2;
		}
	}
	return NULL;
}

Element * collapse(Element * e) {
	switch (e->type) {
		case QUAD:
			return collapse_quad(e);
		case HEXA:
			return collapse_hexa(e);
	}
	NotImplemented("Collapse for ElType");
	return NULL;
};
