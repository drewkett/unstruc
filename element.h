struct Point;

enum Shapes { LINE = 3,
			  TRI = 5,
			  QUAD = 9,
			  TETRA = 10,
			  HEXA = 12,
			  WEDGE = 13,
			  PYRAMID = 14};

struct Element
{
	int type;
	int len;
	int name_i;
	int i;
	Point * s;
	Point *** points;
	Element(int T,int N,Point *** pts);
	Element(int T);
};

bool is2D(Element * e);
bool is3D(Element * e);
void dump(Element * e);
void set_s(Element * e);
bool compareElement(Element * e1, Element * e2);
bool compareElementByName(Element * e1, Element * e2);
bool close(Element * e1, Element * e2);
bool same(Element * e1, Element * e2);

Element * triFromQuad(Element * e,int i1, int i2, int i3);

Element * pyramidFromHexa(Element * e,int i1, int i2, int i3, int i4, int i5);
Element * wedgeFromHexa(Element * e,int i1, int i2, int i3, int i4, int i5, int i6);

bool canCollapse(Element * e);

Element * collapseTri(Element * e);
Element * collapseQuad(Element * e);
Element * collapsePyramid(Element * e);
Element * collapseWedge(Element * e) ;
Element * collapseHexa(Element * e);
Element * collapse(Element * e);
