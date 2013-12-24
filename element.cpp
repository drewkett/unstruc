#include "element.h"
#include "point.h"
#include "error.h"

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
			points = new Point**[len];
			break;
		case TRI:
			len = 3;
			dim = 2;
			points = new Point**[len];
			break;
		case QUAD:
			len = 4;
			dim = 2;
			points = new Point**[len];
			break;
		case HEXA:
			len = 8;
			dim = 3;
			points = new Point**[len];
			break;
		case WEDGE:
			len = 6;
			dim = 3;
			points = new Point**[len];
			break;
		case PYRAMID:
			len = 5;
			dim = 3;
			points = new Point**[len];
			break;
		default:
			NotImplemented("Element Type");
	}
}

void dump(Element * e) {
	std::cerr << e << " -> Element " << e->type << std::endl;
	for (int i=0; i < e->len; i++) {
		dump(e->points[i]);
	}
};

void set_s_by_lowest_id(Element * e) {
	if (!e) return;
	e->s = (*e->points[0]);
	for (int i=1; i < e->len; i++) {
		if ((*e->points[i]) < e->s) {
			e->s = *e->points[i];
		}
	}
}

bool compare_element(Element * e1, Element * e2) {
	if (!e1) return false;
	if (!e2) return true;
	return e1->s < e2->s;
}

bool compare_element_by_name(Element * e1, Element * e2) {
	if (!e1) return false;
	if (!e2) return true;
	return e1->name_i < e2->name_i;
}

bool compare_element_by_index(Element * e1, Element * e2) {
	if (!e1) return false;
	if (!e2) return true;
	return e1->i < e2->i;
}

bool close(Element * e1, Element * e2) {
	return e1->s == e2->s;
};

bool same(Element * e1, Element * e2) {
	if (e1->type != e2->type) return false;
	bool br;
	for (int i = 0; i < e1->len; i++) {
		br = false;
		for (int j = 0; j < e2->len; j++) {
			if (*e1->points[i] == *e2->points[j]) {
				br = true;
				break;
			}
		}
		if (!br) return false;
	}
	for (int j = 0; j < e2->len; j++) {
		br = false;
		for (int i = 0; i < e1->len; i++) {
			if (*e1->points[i] == *e2->points[j]) {
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
			dump(e);
			throw 1;
		}
		collapse_tri(e);
	}
	return NULL;
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

Element * collapse_pyramid(Element * e) {
	if (e->type != PYRAMID) WrongElement(e->type,PYRAMID);
	if (canCollapse(e)) NotImplemented("_pyramid Collapse");
	return NULL;
}

Element * collapse_wedge(Element * e) {
	if (e->type != WEDGE) WrongElement(e->type,WEDGE);
	if (canCollapse(e)) NotImplemented("_wedge Collapse");
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
