
#include "openfoam.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"
#include "vtk.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>
#include <cstring>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

enum OFCellType {
	OFUnknown,
	OFHexa,
	OFWedge,
	OFPrism,
	OFPyramid,
	OFTetra,
	OFTetraWedge,
	OFPoly
};

struct FoamHeader {
	std::string version;
	std::string format;
	std::string _class;
	std::string location;
	std::string object;
	std::string note;
	std::string filename;
};

struct OFBoundary {
	std::string name;
	int n_faces;
	int start_face;
};

struct OFFace {
	bool is_finished;
	bool is_tri_split;
	bool center_calculated;
	bool center_id_assigned;
	int face_center_id;
	double area;
	std::vector<int> points;
	Vector normal;
	Point center;
	std::vector<OFFace> split_faces;
	OFFace () : is_finished(false), is_tri_split(false), center_calculated(false), center_id_assigned(false) {};
};

struct OFInfo {
	int n_points, n_cells, n_faces, n_internal_faces;
};

int removeStraightEdges(OFFace& face, Grid& grid) {
	int n_large_angles = 0;
	bool large_angles[face.points.size()];
	for (int i = 0; i < face.points.size(); ++i) {
		Point& p0 = grid.points[face.points[i]];
		Point& p1 = grid.points[face.points[((i-1)+face.points.size())%face.points.size()]];
		Point& p2 = grid.points[face.points[(i+1)%face.points.size()]];
		Vector v1 = p1 - p0;
		Vector v2 = p2 - p0;
		double angle = angle_between(v2,v1);
		assert (angle == angle);
		assert (angle >= 0);
		large_angles[i] = (angle > 179);
		if (large_angles[i]) n_large_angles++;
	}
	if (n_large_angles == 0) return 0;

	std::vector<int> new_points;
	if (face.points.size() - n_large_angles < 3) {
		Fatal("angles");
	} else {
		for (int i = 0; i < face.points.size(); ++i) {
			if (large_angles[i]) continue;
			new_points.push_back(face.points[i]);
		}
	}
	face.points = new_points;
	return n_large_angles;
}

Point calcCellCenter(std::vector<OFFace*> faces, Grid& grid, int n_owners) {
	// Calculate temp_center which is a rough guess at the center of the cell
	Point temp_center;
	double total_area = 0;
	for (OFFace* face : faces) {
		temp_center.x += face->center.x * face->area;
		temp_center.y += face->center.y * face->area;
		temp_center.z += face->center.z * face->area;
		total_area += face->area;
	}
	temp_center.x /= total_area;
	temp_center.y /= total_area;
	temp_center.z /= total_area;

	Point cell_center;
	double total_volume = 0;
	for (int j = 0; j < faces.size(); ++j) {
		bool faces_out = (j < n_owners);
		OFFace* face = faces[j];
		int n = face->points.size();
		double face_volume = 0;
		for (int k1 = 0; k1 < n; ++k1) {
			int k2 = (k1 + 1) % n;

			Point& p1 = grid.points[face->points[k1]];
			Point& p2 = grid.points[face->points[k2]];

			Vector v1 = p1 - face->center;
			Vector v2 = p2 - p1;
			Vector v3 = temp_center - face->center;
			double volume = dot(v3,cross(v1,v2))/6;
			if (faces_out) volume *= -1;

			face_volume += volume;
			total_volume += volume;
			cell_center.x += volume*(p1.x + p2.x + face->center.x + temp_center.x)/4;
			cell_center.y += volume*(p1.y + p2.y + face->center.y + temp_center.y)/4;
			cell_center.z += volume*(p1.z + p2.z + face->center.z + temp_center.z)/4;
		}
	}
	cell_center.x /= total_volume;
	cell_center.y /= total_volume;
	cell_center.z /= total_volume;
	return cell_center;
}

std::vector<OFFace> splitPolyFace(OFFace& face, Grid& grid, bool debug) {
	if (face.points.size() < 5) Fatal("(openfoam.cpp::splitFace) Not a PolyFace");
	std::vector<OFFace> split_faces;

	double max_angle = 0;
	int max_i = -1;
	int n_points = face.points.size();
	for (int i = 0; i < n_points; ++i) {
		Point& p0 = grid.points[face.points[i]];
		Point& p1 = grid.points[face.points[((i-1)+n_points)%n_points]];
		Point& p2 = grid.points[face.points[(i+1)%n_points]];
		Vector v1 = p1 - p0;
		Vector v2 = p2 - p0;
		double angle = angle_between(v2,v1);
		assert (angle == angle);
		assert (angle >= 0);
		if (angle > max_angle) {
			max_angle = angle;
			max_i = i;
		}
		if (debug)
			printf("%6.1f",angle);
	}
	assert (max_i != -1);

	Point& p0 = grid.points[face.points[max_i]];
	Point& p1 = grid.points[face.points[(max_i+1)%n_points]];
	Vector v1 = p1 - p0;
	double min_diff = 180;
	int min_i = -1;
	for (int i_offset = 2; i_offset < n_points-1; ++i_offset) {
		int i = (max_i + i_offset) % n_points;
		Point& p2 = grid.points[face.points[i]];
		Vector v2 = p2 - p0;
		double diff = fabs(max_angle/2 - angle_between(v1,v2));
		if (diff < min_diff) {
			min_diff = diff;
			min_i = i;
		}
	}
	assert (min_i != -1);
	if (debug)
		printf(" => %d, %d\n",max_i,min_i);
	OFFace face1;
	int face1_points = ((min_i - max_i + 1) + n_points) % n_points;
	for (int i_off = 0; i_off < face1_points; ++i_off) {
		int i = (max_i + i_off) % n_points;
		face1.points.push_back(face.points[i]);
	}

	assert (face1_points > 2);
	if (face1_points == 3 || face1_points == 4) {
		split_faces.push_back(face1);
	} else {
		std::vector<OFFace> new_split_faces = splitPolyFace(face1,grid,debug);
		split_faces.insert(split_faces.end(),new_split_faces.begin(),new_split_faces.end());
	}

	OFFace face2;
	int face2_points = ((max_i - min_i + 1) + n_points) % n_points;
	for (int i_off = 0; i_off < face2_points; ++i_off) {
		int i = (min_i + i_off) % n_points;
		face2.points.push_back(face.points[i]);
	}

	assert (face2_points > 2);
	if (face2_points == 3 || face2_points == 4) {
		split_faces.push_back(face2);
	} else {
		std::vector<OFFace> new_split_faces = splitPolyFace(face2,grid,debug);
		split_faces.insert(split_faces.end(),new_split_faces.begin(),new_split_faces.end());
	}
	return split_faces;
}


void calcFaceCenter(OFFace& face, Grid& grid) {
	if (face.center_calculated) return;

	int n = face.points.size();
	switch (n) {
		case 0:
		case 1:
		case 2:
			Fatal("calcFaceCenter passed face with less than 3 points");
		case 3:
			{
				Point& p0 = grid.points[face.points[0]];
				Point& p1 = grid.points[face.points[1]];
				Point& p2 = grid.points[face.points[2]];

				face.center.x = (p0.x + p1.x + p2.x)/n;
				face.center.y = (p0.y + p1.y + p2.y)/n;
				face.center.z = (p0.z + p1.z + p2.z)/n;

				Vector v1 = p1 - p0;
				Vector v2 = p2 - p1;
				face.normal = cross(v1,v2)/2;
				face.area = face.normal.length();
				face.center_calculated = true;
			}
			break;
		case 4:
			{
				Point& p0 = grid.points[face.points[0]];
				Point& p1 = grid.points[face.points[1]];
				Point& p2 = grid.points[face.points[2]];
				Point& p3 = grid.points[face.points[3]];

				double total_length = 0;
				for (int i1 = 0; i1 < n; ++i1) {
					int i2 = (i1 + 1) % n;
					Point& p1 = grid.points[face.points[i1]];
					Point& p2 = grid.points[face.points[i2]];

					Vector v = p1 - p2;
					double l = v.length();
					total_length += l;
					face.center.x += (p1.x+p2.x)*l/2;
					face.center.y += (p1.y+p2.y)*l/2;
					face.center.z += (p1.z+p2.z)*l/2;
				}
				face.center.x /= total_length;
				face.center.y /= total_length;
				face.center.z /= total_length;

				Vector v02 = p2 - p0;
				Vector v13 = p3 - p1;
				face.normal = cross(v13,v02)/2;
				face.area = face.normal.length();
				face.center_calculated = true;
			}
			break;
		default:
			{
				Point temp_center;
				double total_length = 0;
				for (int i1 = 0; i1 < n; ++i1) {
					int i2 = (i1 + 1) % n;
					Point& p1 = grid.points[face.points[i1]];
					Point& p2 = grid.points[face.points[i2]];

					Vector v = p1 - p2;
					double l = v.length();
					total_length += l;
					temp_center.x += (p1.x+p2.x)*l/2;
					temp_center.y += (p1.y+p2.y)*l/2;
					temp_center.z += (p1.z+p2.z)*l/2;
				}
				temp_center.x /= total_length;
				temp_center.y /= total_length;
				temp_center.z /= total_length;

				double total_area = 0;
				for (int i1 = 0; i1 < n; ++i1) {
					int i2 = (i1 + 1) % n;
					Point& p1 = grid.points[face.points[i1]];
					Point& p2 = grid.points[face.points[i2]];

					Vector v1 = p1 - temp_center;
					Vector v2 = p2 - p1;
					Vector n = cross(v1,v2);
					double area = n.length()/2;

					total_area += area;
					face.center.x += area*(p1.x + p2.x + temp_center.x)/3;
					face.center.y += area*(p1.y + p2.y + temp_center.y)/3;
					face.center.z += area*(p1.z + p2.z + temp_center.z)/3;
					face.normal += n/2;
				}
				face.area = face.normal.length();
				face.center.x /= total_area;
				face.center.y /= total_area;
				face.center.z /= total_area;
				face.center_calculated = true;
			}
			break;
	}
}

void triangleFaceSplit(OFFace* face) {
	assert (face->center_calculated);
	assert (face->center_id_assigned);
	face->split_faces.resize(face->points.size());
	for (int j1 = 0; j1 < face->points.size(); ++j1) {
		int j2 = (j1 + 1) % face->points.size();
		OFFace new_face;
		new_face.points.push_back(face->points[j1]);
		new_face.points.push_back(face->points[j2]);
		new_face.points.push_back(face->face_center_id);
		face->split_faces[j1] = new_face;
	}
}

double calcQuadWarp(Grid& grid, int _p1, int _p2, int _p3, int _p4) {
	Point& p1 = grid.points[_p1];
	Point& p2 = grid.points[_p2];
	Point& p3 = grid.points[_p3];
	Point& p4 = grid.points[_p4];
	Vector v13 = p3 - p1;
	Vector v24 = p4 - p2;
	Vector n = cross(v13,v24);
	double area = n.length()/2;

	assert (area > 0);
	Vector v12 = p2 - p1;
	double volume = 0.25*dot(n,v12);
	double height = volume/area;
	double length = sqrt(area);
	return height/length;
}

void createElementsFromFaceCenter(OFFace& face, bool faces_out, int center_id, std::vector<Element>& new_elements) {
	assert (face.points.size() > 2);
	if (face.points.size() == 3 && !face.is_tri_split) {
		// If current face only has 3 points, create a tetrahedral with face plus cell center
		Element e (TETRA);
		e.name_i = 0;

		if (faces_out) {
			e.points[2] = face.points[0];
			e.points[1] = face.points[1];
			e.points[0] = face.points[2];
		} else {
			e.points[0] = face.points[0];
			e.points[1] = face.points[1];
			e.points[2] = face.points[2];
		}
		e.points[3] = center_id;
		new_elements.push_back(e);
	} else if (face.points.size() == 4 && !face.is_tri_split) {
		// If current face only has 4 points, create a pyramid with face plus cell center
		Element e (PYRAMID);

		if (faces_out) {
			e.points[3] = face.points[0];
			e.points[2] = face.points[1];
			e.points[1] = face.points[2];
			e.points[0] = face.points[3];
		} else {
			e.points[0] = face.points[0];
			e.points[1] = face.points[1];
			e.points[2] = face.points[2];
			e.points[3] = face.points[3];
		}
		e.points[4] = center_id;
		new_elements.push_back(e);
	} else {
		assert (face.split_faces.size());
		for (OFFace& new_face : face.split_faces) {
			createElementsFromFaceCenter(new_face, faces_out, center_id, new_elements);
		}
	}
}
bool createWedgeElementsFromSideFace(Grid& grid, OFFace* side_face, OFFace* main_face, OFFace* opp_face, bool side_faces_out, int main_face_center_id, int opp_face_center_id, std::vector<Element>& new_elements) {
	bool pt_on_main_face [side_face->points.size()];
	int n_on_main_face = 0;
	for (int _p = 0; _p < side_face->points.size(); ++_p) {
		int p = side_face->points[_p];
		auto it = std::find(main_face->points.begin(),main_face->points.end(),p);
		pt_on_main_face[_p] = it != main_face->points.end();
		if (pt_on_main_face[_p])
			n_on_main_face++;
	}
	if (side_face->points.size() == 3 && !side_face->is_tri_split) {
		return false;
		int start;
		if (n_on_main_face == 2) {
			if (!pt_on_main_face[0])
				start = 0;
			else if (!pt_on_main_face[1])
				start = 1;
			else
				start = 2;

			int _p0, _p1, _p2;
			if (side_faces_out) {
				_p0 = (start + 0) % 3;
				_p1 = (start + 1) % 3;
				_p2 = (start + 2) % 3;
			} else {
				_p0 = (start - 0 + 3) % 3;
				_p1 = (start - 1 + 3) % 3;
				_p2 = (start - 2 + 3) % 3;
			}

			std::vector<OFFace *> new_faces;

			calcFaceCenter(*side_face,grid);
			new_faces.push_back(side_face);

			OFFace face0;
			face0.points.push_back(main_face_center_id);
			face0.points.push_back(side_face->points[_p1]);
			face0.points.push_back(side_face->points[_p2]);
			calcFaceCenter(face0,grid);
			new_faces.push_back(&face0);

			OFFace face1;
			face1.points.push_back(opp_face_center_id);
			face1.points.push_back(main_face_center_id);
			face1.points.push_back(side_face->points[_p2]);
			face1.points.push_back(side_face->points[_p0]);
			calcFaceCenter(face1,grid);
			new_faces.push_back(&face1);

			OFFace face2;
			face2.points.push_back(main_face_center_id);
			face2.points.push_back(opp_face_center_id);
			face2.points.push_back(side_face->points[_p0]);
			face2.points.push_back(side_face->points[_p1]);
			calcFaceCenter(face2,grid);
			new_faces.push_back(&face2);

			int n_owners = side_faces_out;
			Point cell_center = calcCellCenter(new_faces,grid,n_owners);

			int cell_center_id = grid.points.size();
			grid.points.push_back(cell_center);

			createElementsFromFaceCenter(*side_face,side_faces_out,cell_center_id,new_elements);
			createElementsFromFaceCenter(face0,false,cell_center_id,new_elements);
			createElementsFromFaceCenter(face1,false,cell_center_id,new_elements);
			createElementsFromFaceCenter(face2,false,cell_center_id,new_elements);

			//new_elements.emplace_back(PYRAMID);
			//Element& e = new_elements.back();
			//e.points[0] = main_face_center_id;
			//e.points[1] = opp_face_center_id;
			//for (int _p = 0; _p < 3; ++_p) {
			//	int p;
			//	if (side_faces_out)
			//		p = (start + _p) % 3;
			//	else
			//		p = (start - _p + 3) % 3;
			//	e.points[2+_p] = side_face->points[p];
			//}
			//double warp = calcQuadWarp(grid, e.points[0],e.points[1],e.points[2],e.points[3]);
			//double warp_other = calcQuadWarp(grid, e.points[0],e.points[1],e.points[2],e.points[4]);
			//if (fabs(warp_other) < fabs(warp)) {
			//	int temp1 = e.points[1];
			//	e.points[1] = e.points[4];
			//	e.points[4] = e.points[3];
			//	e.points[3] = temp1;
			//	//printf("Correcting pyramid based on warp\n");
			//}
		} else if (n_on_main_face == 1) {
			return false;
			if (pt_on_main_face[0])
				start = 0;
			else if (pt_on_main_face[1])
				start = 1;
			else
				start = 2;

			int _p0, _p1, _p2;
			if (side_faces_out) {
				_p0 = (start + 0) % 3;
				_p1 = (start + 1) % 3;
				_p2 = (start + 2) % 3;
			} else {
				_p0 = (start - 0 + 3) % 3;
				_p1 = (start - 1 + 3) % 3;
				_p2 = (start - 2 + 3) % 3;
			}

			std::vector<OFFace *> new_faces;

			calcFaceCenter(*side_face,grid);
			new_faces.push_back(side_face);

			OFFace main_face;
			main_face.points.push_back(opp_face_center_id);
			main_face.points.push_back(side_face->points[_p1]);
			main_face.points.push_back(side_face->points[_p2]);
			calcFaceCenter(main_face,grid);
			new_faces.push_back(&main_face);

			OFFace face1;
			face1.points.push_back(main_face_center_id);
			face1.points.push_back(opp_face_center_id);
			face1.points.push_back(side_face->points[_p2]);
			face1.points.push_back(side_face->points[_p0]);
			calcFaceCenter(face1,grid);
			new_faces.push_back(&face1);

			OFFace face2;
			face2.points.push_back(opp_face_center_id);
			face2.points.push_back(main_face_center_id);
			face2.points.push_back(side_face->points[_p0]);
			face2.points.push_back(side_face->points[_p1]);
			calcFaceCenter(face2,grid);
			new_faces.push_back(&face2);

			int n_owners = side_faces_out;
			Point cell_center = calcCellCenter(new_faces,grid,n_owners);

			int cell_center_id = grid.points.size();
			grid.points.push_back(cell_center);

			createElementsFromFaceCenter(*side_face,side_faces_out,cell_center_id,new_elements);
			createElementsFromFaceCenter(main_face,false,cell_center_id,new_elements);
			createElementsFromFaceCenter(face1,false,cell_center_id,new_elements);
			createElementsFromFaceCenter(face2,false,cell_center_id,new_elements);

			bool fail = false;
			for (Element& e: new_elements)
				if (e.calc_volume(grid) < -1e-5)
					fail = true;
			if (fail) {
				for (Element& e: new_elements) {
					dump(e,grid);
					printf("Volume = %g\n",e.calc_volume(grid));
				}
				Fatal();
			}

			//if (pt_on_main_face[0])
			//	start = 0;
			//else if (pt_on_main_face[1])
			//	start = 1;
			//else
			//	start = 2;
			//new_elements.emplace_back(PYRAMID);
			//Element& e = new_elements.back();
			//e.points[0] = opp_face_center_id;
			//e.points[1] = main_face_center_id;
			//for (int _p = 0; _p < 3; ++_p) {
			//	int p;
			//	if (side_faces_out)
			//		p = (start + _p) % 3;
			//	else
			//		p = (start - _p + 3) % 3;
			//	e.points[2+_p] = side_face->points[p];
			//}
			//double warp = calcQuadWarp(grid, e.points[0],e.points[1],e.points[2],e.points[3]);
			//double warp_other = calcQuadWarp(grid, e.points[0],e.points[1],e.points[2],e.points[4]);
			//if (fabs(warp_other) < fabs(warp)) {
			//	int temp1 = e.points[1];
			//	e.points[1] = e.points[4];
			//	e.points[4] = e.points[3];
			//	e.points[3] = temp1;
			//	//printf("Correcting pyramid based on warp\n");
			//}
		} else {
			return false;
		}
	} else if (side_face->points.size() == 4 && !side_face->is_tri_split) {
		int p0 = side_face->points[0];
		int p1 = side_face->points[1];
		auto it0 = std::find(main_face->points.begin(),main_face->points.end(),p0);
		auto it1 = std::find(main_face->points.begin(),main_face->points.end(),p1);
		bool p0_on_main_face = it0 != main_face->points.end();
		bool p1_on_main_face = it1 != main_face->points.end();
		int order[4];
		if (p0_on_main_face && p1_on_main_face){
			if (side_faces_out) {
				order[0] = 1;
				order[1] = 0;
				order[2] = 2;
				order[3] = 3;
			} else {
				order[0] = 0;
				order[1] = 1;
				order[2] = 3;
				order[3] = 2;
			}
		} else if (!p0_on_main_face && p1_on_main_face){
			if (side_faces_out) {
				order[0] = 2;
				order[1] = 1;
				order[2] = 3;
				order[3] = 0;
			} else {
				order[0] = 1;
				order[1] = 2;
				order[2] = 0;
				order[3] = 3;
			}
		} else if (p0_on_main_face && !p1_on_main_face){
			if (side_faces_out) {
				order[0] = 0;
				order[1] = 3;
				order[2] = 1;
				order[3] = 2;
			} else {
				order[0] = 3;
				order[1] = 0;
				order[2] = 2;
				order[3] = 1;
			}
		} else {
			if (side_faces_out) {
				order[0] = 3;
				order[1] = 2;
				order[2] = 0;
				order[3] = 1;
			} else {
				order[0] = 2;
				order[1] = 3;
				order[2] = 1;
				order[3] = 0;
			}
		}
		new_elements.emplace_back(WEDGE);
		Element& e = new_elements.back();
		e.points[0] = main_face_center_id;
		e.points[1] = side_face->points[order[0]];
		e.points[2] = side_face->points[order[1]];
		e.points[3] = opp_face_center_id;
		e.points[4] = side_face->points[order[2]];
		e.points[5] = side_face->points[order[3]];
	} else {
		assert (side_face->split_faces.size());
		for (OFFace& split_face : side_face->split_faces) {
			bool success = createWedgeElementsFromSideFace(grid,&split_face,main_face,opp_face,side_faces_out,main_face_center_id,opp_face_center_id,new_elements);
			if (!success)
				return false;
		}
	}
	return true;
}

FoamHeader readFoamHeader(std::ifstream& f, std::string filename) {
	std::string line;
	bool is_foamfile = false;
	while (std::getline(f,line)) {
		if (line.compare(0,8,"FoamFile") == 0) {
			is_foamfile = true;
			break;
		}
	}
	if (!is_foamfile) Fatal("Not a valid FoamFile");

	std::string token;
	f >> token;
	if (token != "{") Fatal("Not a valid FoamFile");

	FoamHeader header;
	header.filename = filename;
	f >> token;
	while (token != "}") {
		if (token == "version") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.version = token;
		} else if (token == "format") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.format = token;
		} else if (token == "class") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header._class = token;
		} else if (token == "location") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.location = token;
		} else if (token == "object") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.object = token;
		} else if (token == "note") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.note = token;
		} else {
			std::cerr << token << std::endl;
			Fatal("Not a valid FoamFile");
		}
		f >> token;
	}

	return header;
}

int readNextInt(std::ifstream& f) {
	int n = -1;
	std::string line;
	while (std::getline(f,line)) {
		if (line.size() == 0) continue;
		if (line[0] == '/' && line[1] == '/') continue;
		n = atoi(line.c_str());
		break;
	}
	if (n == -1) Fatal("Invalid FoamFile");
	return n;
}

template<typename T>
std::vector<T> readBinary(std::ifstream& f, FoamHeader& header) {
	int n = readNextInt(f);

	std::vector<T> vec;
	vec.resize(n);

	if (n) {
		char c = f.get();
		if (c != '(') Fatal("Invalid FoamFile: Expected '(' in "+header.filename);

		for (int i = 0; i < n; ++i) {
			f.read((char *) &vec[i], sizeof(T));
		}
		c = f.get();
		if (c != ')') Fatal("Invalid FoamFile: Expected ')' in "+header.filename);
	}
	
	return vec;
}

OFInfo readInfoFromOwners(std::string polymesh) {
	std::string filepath = polymesh + "/owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"owner");

	std::string note = header.note.substr(1,header.note.size()-2);
	std::istringstream ss(note);
	std::string token;
	std::string name;
	int i;

	OFInfo info = {-1, -1, -1, -1};

	while (ss >> token) {
		i = token.find_first_of(':');
		if (token.compare(0,i,"nPoints") == 0) {
			if (i < token.size()-1)
				info.n_points = atoi(token.substr(i+1).c_str());
			else {
				ss >> token;
				info.n_points = atoi(token.c_str());
			}
		} else if (token.compare(0,i,"nCells") == 0) {
			if (i < token.size()-1)
				info.n_cells = atoi(token.substr(i+1).c_str());
			else {
				ss >> token;
				info.n_cells = atoi(token.c_str());
			}
		} else if (token.compare(0,i,"nFaces") == 0) {
			if (i < token.size()-1)
				info.n_faces = atoi(token.substr(i+1).c_str());
			else {
				ss >> token;
				info.n_faces = atoi(token.c_str());
			}
		} else if (token.compare(0,i,"nInternalFaces") == 0) {
			if (i < token.size()-1)
				info.n_internal_faces = atoi(token.substr(i+1).c_str());
			else {
				ss >> token;
				info.n_internal_faces = atoi(token.c_str());
			}
		} else {
			Fatal("Invalid FoamFile: Unknown key in header.note "+token.substr(0,i));
		}
	}
	if (info.n_points == -1) Fatal("Invalid FoamFile: nPoints not found in owner.note");
	if (info.n_cells == -1) Fatal("Invalid FoamFile: nCells not found in owner.note");
	if (info.n_faces == -1) Fatal("Invalid FoamFile: nFaces not found in owner.note");
	if (info.n_internal_faces == -1) Fatal("Invalid FoamFile: nInternalFaces not found in owner.note");
	return info;
}

std::vector<int> readOwners(const std::string polymesh) {
	std::string filepath = polymesh + "/owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"owner");

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<int> readNeighbours(const std::string& polymesh) {
	std::string filepath = polymesh + "/neighbour";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"neighbour");

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<Point> readPoints(const std::string& polymesh) {
	std::string filepath = polymesh + "/points";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"points");

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<Point>(f,header);
}

std::vector<OFFace> readFaces(const std::string& polymesh) {
	std::string filepath = polymesh + "/faces";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"faces");

	if (header.format == "ascii") Fatal("ascii not supported");

	std::vector<OFFace> faces;
	std::vector<int> index = readBinary<int>(f,header);
	std::vector<int> points = readBinary<int>(f,header);

	faces.resize(index.size() - 1);
	for (int i = 0; i < index.size() - 1; ++i)
		faces[i].points = std::vector<int> (&points[index[i]],&points[index[i+1]]);

	return faces;
}

std::vector<OFBoundary> readBoundaries(const std::string& polymesh) {
	std::string filepath = polymesh + "/boundary";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f,"boundary");

	int n = readNextInt(f);

	std::string token;
	f >> token;
	if (token != "(") Fatal("Invalid FoamFile: Expected '(' in "+header.location);

	std::vector<OFBoundary> boundaries;
	boundaries.resize(n);

	for (int i = 0; i < n; ++i) {
		f >> boundaries[i].name;
		boundaries[i].n_faces = -1;
		boundaries[i].start_face = -1;
		f >> token;
		if (token != "{") Fatal("Invalid FoamFile: Expected '{' in "+header.location);
		while (token != "}") {
			if (token == "nFaces") {
				std::getline(f,token,';');
				token.erase(0,token.find_first_not_of(' '));
				boundaries[i].n_faces = atoi(token.c_str());
			} else if (token == "startFace") {
				std::getline(f,token,';');
				token.erase(0,token.find_first_not_of(' '));
				boundaries[i].start_face = atoi(token.c_str());
			}
			f >> token;
		}
		if (boundaries[i].n_faces == -1) Fatal("Invalid FoamFile: Expected nFaces for patch "+boundaries[i].name);
		if (boundaries[i].start_face == -1) Fatal("Invalid FoamFile: Expected startFace for patch "+boundaries[i].name);
	}

	f >> token;
	if (token != ")") Fatal("Invalid FoamFile: Expected ')' in "+header.location);

	return boundaries;
}
OFCellType determineCellType (std::vector<OFFace*>& faces) {
	OFCellType cell_type = OFUnknown;
	int n_tri = 0;
	int n_quad = 0;
	int n_poly = 0;
	for (OFFace* face : faces) {
		switch (face->points.size()) {
			case 0:
			case 1:
			case 2:
				Fatal("Bad Face");
			case 3:
				n_tri++;
				break;
			case 4:
				n_quad++;
				break;
			default:
				n_poly++;
				break;
		}
	}
	switch (faces.size()) {
		case 0:
		case 1:
		case 2:
		case 3:
			Fatal("Bad Cell");
		case 4:
			if (n_tri == 4)
				return OFTetra;
			else if (n_quad == 2 && n_tri == 2)
				return OFTetraWedge;
			else
				return OFPoly;
		case 5:
			if (n_quad == 3 && n_tri == 2)
				return OFPrism;
			else if (n_quad == 1 && n_tri == 4)
				return OFPyramid;
			else
				return OFPoly;
		case 6:
			if (n_quad == 6)
				return OFHexa;
			else if (n_quad == 4 && n_tri == 2)
				return OFWedge;
			else
				return OFPoly;
		default:
			return OFPoly;
	}
	if (cell_type == OFUnknown) {
		printf("nFaces %zu nTri %d nQuad %d\n",faces.size(),n_tri,n_quad);
		Fatal("Unknown Cell Type");
	}
	return OFUnknown;
}

void readOpenFoam(Grid& grid, std::string &polymesh) {
	struct stat s;
	if (! (stat(polymesh.c_str(),&s) == 0 && (s.st_mode & S_IFDIR)) )
		Fatal(polymesh + " isn't a directory");
	
	OFInfo info = readInfoFromOwners(polymesh);
	grid.points = readPoints(polymesh);
	std::vector<OFFace> faces = readFaces(polymesh);
	std::vector<int> owners = readOwners(polymesh);
	std::vector<int> neighbours = readNeighbours(polymesh);
	std::vector<OFBoundary> boundaries = readBoundaries(polymesh);

	grid.dim = 3;

	printf("Points: %d\nFaces: %d\nInternal Faces: %d\nCells: %d\n",info.n_points,info.n_faces,info.n_internal_faces,info.n_cells);
	if (info.n_points != grid.points.size()) Fatal("Invalid FoamFile: number of points do not match");

	int name_i = 0;
	grid.names.emplace_back(3,"default");

	if (info.n_faces != faces.size() ) Fatal("Invalid FoamFile: number of faces do not match");

	for (OFFace& face : faces) {
		calcFaceCenter(face,grid);
		if (face.points.size() > 4) 
			face.split_faces = splitPolyFace(face,grid,false);
	}

	std::vector< int > n_faces_per_cell (info.n_cells,0);
	std::vector< int > n_owners_per_cell (info.n_cells,0);
	std::vector< std::vector<OFFace*> > faces_per_cell (info.n_cells);
	for (int i = 0; i < owners.size(); ++i) {
		n_faces_per_cell[owners[i]]++;
		n_owners_per_cell[owners[i]]++;
		faces_per_cell[owners[i]].push_back(&faces[i]);
	}
	for (int i = 0; i < neighbours.size(); ++i) {
		n_faces_per_cell[neighbours[i]]++;
		faces_per_cell[neighbours[i]].push_back(&faces[i]);
	}

	int n_wedge_split = 0;
	int n_wedge_split_failed = 0;
	std::vector< OFCellType > cell_types (info.n_cells,OFUnknown);
	for (int i = 0; i < info.n_cells; ++i) {
		std::vector<OFFace*>& cell_faces = faces_per_cell[i];
		cell_types[i] = determineCellType(cell_faces);
	}

	std::vector< Element > failed_split_elements;
	std::vector< bool > processed_cells (info.n_cells,false);
	for (int i = 0; i < info.n_cells; ++i) {
		std::vector<OFFace*>& cell_faces = faces_per_cell[i];
		OFCellType cell_type = cell_types[i];
		if (cell_type != OFPoly) continue;

		// Find complete set of points that make up cell by doing repeated unions
		std::vector<int> point_set (0);
		std::sort(point_set.begin(),point_set.end());

		for (OFFace* face : cell_faces) {

			// create vector of points for current face
			std::vector<int> current_set (face->points);
			std::sort(current_set.begin(),current_set.end());

			// copy point_set to temp_set so that final union goes back in point_set
			std::vector<int> temp_set (point_set);

			// add space for union to add new values to point_set
			point_set.resize(current_set.size() + temp_set.size());

			std::vector<int>::iterator it;
			it = std::set_union(temp_set.begin(),temp_set.end(),current_set.begin(),current_set.end(),point_set.begin());
			point_set.resize(it-point_set.begin());
		}

		for (int j = 0; j < cell_faces.size()-1; ++j) {
			OFFace *face = cell_faces[j];
			if (face->points.size() < 5) continue;
			if (face->is_finished) continue;
			for (int k = j+1; k < cell_faces.size(); ++k) {
				OFFace *other_face = cell_faces[k];
				if (other_face->is_finished) continue;
				bool share_points = false;
				for (int p : face->points)
					for (int po : other_face->points)
						share_points |= (p == po);
				if (share_points) continue;
				if (face->points.size() + other_face->points.size() == point_set.size()) {
					bool side_face_is_tri_split = false;
					for (int l = 0; l < cell_faces.size(); ++l) {
						if (l == j || l == k) continue;
						OFFace* side_face = cell_faces[l];
						side_face_is_tri_split |= side_face->is_tri_split;
					}
					if (side_face_is_tri_split)
						break;

					int face_center_id;
					if (face->is_tri_split) {
						face_center_id = face->face_center_id;
					} else {
						face_center_id = grid.points.size();
						grid.points.push_back(face->center);
					}

					int other_face_center_id;
					if (other_face->is_tri_split) {
						other_face_center_id = other_face->face_center_id;
					} else {
						other_face_center_id = grid.points.size();
						grid.points.push_back(other_face->center);
					}

					std::vector<Element> new_elements;
					for (int l = 0; l < cell_faces.size(); ++l) {
						if (l == j || l == k) continue;
						OFFace* side_face = cell_faces[l];
						bool side_faces_out = (l < n_owners_per_cell[i]);
						bool success = createWedgeElementsFromSideFace(grid,side_face,face,other_face,side_faces_out,face_center_id,other_face_center_id,new_elements);
						for (Element& e : new_elements) {
							if (e.calc_volume(grid) < -1e-3)
								Fatal("Large negative volume");
							if (e.calc_volume(grid) < 0)
								success = false;
						}
						if (!success) {
							new_elements.clear();
							break;
						} 
					}
					if (new_elements.size() > 0) {
						if (!face->is_tri_split) {
							face->is_tri_split = true;
							face->face_center_id = face_center_id;
							face->center_id_assigned = true;
							//Must assign face center id before splitting
							triangleFaceSplit(face);
						}
						if (!other_face->is_tri_split) {
							other_face->is_tri_split = true;
							other_face->face_center_id = other_face_center_id;
							other_face->center_id_assigned = true;
							//Must assign face center id before splitting
							triangleFaceSplit(other_face);
						}

						for (OFFace* face : cell_faces) {
							face->is_finished = true;
						}

						processed_cells[i] = true;
						n_wedge_split++;
						grid.elements.insert(grid.elements.end(),new_elements.begin(),new_elements.end());
						break;
					} else {
						//Delete unused face centers from grid
						//NOT SURE IF THIS IS SAFE
						//if (!face->split)
						//	grid.points.pop_back();
						//if (!other_face->split)
						//	grid.points.pop_back();

						n_wedge_split_failed++;
						//Add faces to failed_split_elements
						for (OFFace* face : cell_faces) {
							Element e (POLYGON);
							for (int p : face->points)
								e.points.push_back(p);
							failed_split_elements.push_back(e);
						}
					}
				}
			}
			if (processed_cells[i]) break;
		}
	}
	printf("Wedge Split: %d\n",n_wedge_split);
	printf("Failed Wedge Split: %d\n",n_wedge_split_failed);
	if (n_wedge_split_failed) {
		Grid g = grid.grid_from_elements(failed_split_elements);
		toVTK("failed_split_elements.vtk",g,true);		
	}

	for (int i = 0; i < info.n_cells; ++i) {
		std::vector<OFFace*>& cell_faces = faces_per_cell[i];
		OFCellType cell_type = cell_types[i];
		if (processed_cells[i]) continue;

		int n_split_faces = 0;
		for (OFFace* face : cell_faces)
			if (face->split_faces.size())
				n_split_faces++;

		if (n_split_faces) cell_type = OFPoly;

		if (cell_type == OFTetra) {
			grid.elements.emplace_back(TETRA);
			Element& e = grid.elements.back();
			e.name_i = name_i;
			bool faces_out = (n_owners_per_cell[i] > 0);

			OFFace* first_face = cell_faces[0];
			if (faces_out) {
				e.points[2] = first_face->points[0];
				e.points[1] = first_face->points[1];
				e.points[0] = first_face->points[2];
			} else {
				e.points[0] = first_face->points[0];
				e.points[1] = first_face->points[1];
				e.points[2] = first_face->points[2];
			}
			OFFace* second_face = cell_faces[1];
			for (int p2 : second_face->points) {
				bool match = true;
				for (int p1 : first_face->points) {
					if (p2 == p1) {
						match = false;
						break;
					}
				}
				if (match) {
					e.points[3] = p2;
					break;
				}
			}
		//} else if (cell_type == OFTetraWedge) {
		//	int tri1_j = -1;
		//	int tri2_j = -1;
		//	int quad1_j = -1;
		//	for (int j = 0; j < 4; ++j) {
		//		assert (!cell_faces[j]->split);
		//		if (cell_faces[j]->points.size() == 3) {
		//			if (tri1_j == -1)
		//				tri1_j = j;
		//			else if (tri2_j == -1)
		//				tri2_j = j;
		//			else
		//				Fatal("Shouldn't be Possible");
		//		} else {
		//			if (quad1_j == -1)
		//				quad1_j = j;
		//		}
		//	}
		//	assert (tri1_j != tri2_j);
		//	assert (tri1_j != quad1_j);
		//	assert (tri2_j != quad1_j);
		//	assert (tri1_j != -1);
		//	assert (tri2_j != -1);
		//	assert (quad1_j != -1);

		//	bool tri1_faces_out = (tri1_j < n_owners_per_cell[i]);
		//	bool tri2_faces_out = (tri2_j < n_owners_per_cell[i]);

		//	OFFace* tri1_face = cell_faces[tri1_j];
		//	OFFace* tri2_face = cell_faces[tri2_j];
		//	OFFace* quad1_face = cell_faces[quad1_j];

		//	int extra_point = -1;
		//	for (int p : quad1_face->points) {
		//		bool match = false;
		//		for (int tp1 : tri1_face->points) {
		//			if (p == tp1) {
		//				match = true;
		//				break;
		//			}
		//		}
		//		if (match) continue;
		//		for (int tp2 : tri2_face->points) {
		//			if (p == tp2) {
		//				match = true;
		//				break;
		//			}
		//		}
		//		if (!match) {
		//			assert (extra_point == -1);
		//			extra_point = p;
		//		}
		//	}
		//	assert (extra_point != -1);

		//	grid.elements.emplace_back(TETRA);
		//	Element& e1 = grid.elements.back();
		//	e1.name_i = name_i;
		//	if (tri1_faces_out) {
		//		e1.points[2] = tri1_face->points[0];
		//		e1.points[1] = tri1_face->points[1];
		//		e1.points[0] = tri1_face->points[2];
		//	} else {
		//		e1.points[0] = tri1_face->points[0];
		//		e1.points[1] = tri1_face->points[1];
		//		e1.points[2] = tri1_face->points[2];
		//	}
		//	e1.points[3] = extra_point;

		//	grid.elements.emplace_back(TETRA);
		//	Element& e2 = grid.elements.back();
		//	e2.name_i = name_i;
		//	if (tri2_faces_out) {
		//		e2.points[2] = tri2_face->points[0];
		//		e2.points[1] = tri2_face->points[1];
		//		e2.points[0] = tri2_face->points[2];
		//	} else {
		//		e2.points[0] = tri2_face->points[0];
		//		e2.points[1] = tri2_face->points[1];
		//		e2.points[2] = tri2_face->points[2];
		//	}
		//	e2.points[3] = extra_point;
		} else if (cell_type == OFPyramid) {
			grid.elements.emplace_back(PYRAMID);
			Element& e = grid.elements.back();
			e.name_i = name_i;
			int quad_j = -1;
			for (int j = 0; j < 5; ++j) {
				if (cell_faces[j]->points.size() == 4) {
					quad_j = j;
					break;
				}
			}
			if (quad_j == -1) Fatal("PYRAMID: Shouldn't be possible");
			bool faces_out = (quad_j < n_owners_per_cell[i]);

			OFFace* quad_face = cell_faces[quad_j];
			if (faces_out) {
				e.points[3] = quad_face->points[0];
				e.points[2] = quad_face->points[1];
				e.points[1] = quad_face->points[2];
				e.points[0] = quad_face->points[3];
			} else {
				e.points[0] = quad_face->points[0];
				e.points[1] = quad_face->points[1];
				e.points[2] = quad_face->points[2];
				e.points[3] = quad_face->points[3];
			}
			int second_j;
			if (quad_j == 0)
				second_j = 1;
			else
				second_j = 0;
			OFFace* second_face = cell_faces[second_j];
			for (int p : second_face->points) {
				bool match = true;
				for (int p2 : quad_face->points) {
					if (p == p2) {
						match = false;
						break;
					}
				}
				if (match) {
					e.points[4] = p;
					break;
				}
			}
		} else if (cell_type == OFWedge) {
			int tri1_j = -1;
			int tri2_j = -1;
			for (int j = 0; j < 6; ++j) {
				if (cell_faces[j]->points.size() == 3) {
					if (tri1_j == -1)
						tri1_j = j;
					else if (tri2_j == -1)
						tri2_j = j;
					else
						Fatal("WEDGE: Shouldn't be possible");
				}
			}
			if (tri1_j == -1) Fatal("WEDGE: Shouldn't be possible (2)");
			if (tri2_j == -1) Fatal("WEDGE: Shouldn't be possible (3)");
			OFFace* tri1_face = cell_faces[tri1_j];
			OFFace* tri2_face = cell_faces[tri2_j];

			int common_point = -1;
			for (int p1 : tri1_face->points) {
				for (int p2 : tri2_face->points) {
					if (p1 == p2) {
						common_point = p1;
						break;
					}
				}
				if (common_point != -1) break;
			}
			if (common_point == -1) Fatal("WEDGE: Shouldn't be possible (4)");

			int quad1_j = -1;
			int quad2_j = -1;
			for (int j = 0; j < 6; ++j) {
				OFFace* current_face = cell_faces[j];
				if (current_face->points.size() == 3) continue;
				bool match1 = false;
				bool match2 = false;
				for (int p : current_face->points) {
					for (int p1 : tri1_face->points) {
						if (p == p1) {
							match1 = true;
							break;
						}
					}
					for (int p2 : tri2_face->points) {
						if (p == p2) {
							match2 = true;
							break;
						}
					}
					if (match1 && match2) break;
				}
				if (!match1) quad1_j = j;
				if (!match2) quad2_j = j;
			}
			if (quad1_j == -1) Fatal("WEDGE: Shouldn't be possible (5)");
			if (quad2_j == -1) Fatal("WEDGE: Shouldn't be possible (6)");

			bool quad1_faces_out = (quad1_j < n_owners_per_cell[i]);
			bool quad2_faces_out = (quad2_j < n_owners_per_cell[i]);

			OFFace* quad1_face = cell_faces[quad1_j];
			OFFace* quad2_face = cell_faces[quad2_j];

			grid.elements.emplace_back(PYRAMID);
			Element& e1 = grid.elements.back();
			e1.name_i = name_i;
			if (quad1_faces_out) {
				e1.points[3] = quad1_face->points[0];
				e1.points[2] = quad1_face->points[1];
				e1.points[1] = quad1_face->points[2];
				e1.points[0] = quad1_face->points[3];
			} else {
				e1.points[0] = quad1_face->points[0];
				e1.points[1] = quad1_face->points[1];
				e1.points[2] = quad1_face->points[2];
				e1.points[3] = quad1_face->points[3];
			}
			e1.points[4] = common_point;

			grid.elements.emplace_back(PYRAMID);
			Element& e2 = grid.elements.back();
			e2.name_i = name_i;
			if (quad2_faces_out) {
				e2.points[3] = quad2_face->points[0];
				e2.points[2] = quad2_face->points[1];
				e2.points[1] = quad2_face->points[2];
				e2.points[0] = quad2_face->points[3];
			} else {
				e2.points[0] = quad2_face->points[0];
				e2.points[1] = quad2_face->points[1];
				e2.points[2] = quad2_face->points[2];
				e2.points[3] = quad2_face->points[3];
			}
			e2.points[4] = common_point;
		} else if (cell_type == OFPrism) {
			int tri1_j = -1;
			int tri2_j = -1;
			int quad_j = -1;
			for (int j = 0; j < 5; ++j) {
				if (cell_faces[j]->points.size() == 3) {
					if (tri1_j == -1)
						tri1_j = j;
					else if (tri2_j == -1)
						tri2_j = j;
					else
						Fatal("PRISM: Shouldn't be possible");
				} else
					quad_j = j;
			}
			assert (tri1_j != -1);
			assert (tri2_j != -1);
			assert (quad_j != -1);

			bool tri1_faces_out = (tri1_j < n_owners_per_cell[i]);

			OFFace* tri1_face = cell_faces[tri1_j];
			OFFace* tri2_face = cell_faces[tri2_j];
			OFFace* quad_face = cell_faces[quad_j];

			int tri2_points_aligned[3] = {-1, -1, -1};
			for (int j1 = 0; j1 < 4; ++j1) {
				int j2 = (j1 + 1) % 4;

				int p1 = quad_face->points[j1];
				auto it1 = std::find(tri1_face->points.begin(),tri1_face->points.end(),p1);
				bool p1_on_tri1 = (it1 != tri1_face->points.end());

				int p2 = quad_face->points[j2];
				auto it2 = std::find(tri1_face->points.begin(),tri1_face->points.end(),p2);
				bool p2_on_tri1 = (it2 != tri1_face->points.end());

				if (p1_on_tri1 != p2_on_tri1) {
					if (p1_on_tri1) {
						int k = it1 - tri1_face->points.begin();
						tri2_points_aligned[k] = p2;
					} else {
						int k = it2 - tri1_face->points.begin();
						tri2_points_aligned[k] = p1;
					}
				}
			}
			int missing_k;
			for (int k = 0; k < 3; ++k) {
				if (tri2_points_aligned[k] == -1)
					missing_k = k;
			}
			for (int tri2_p : tri2_face->points) {
				bool matched = false;
				for (int p : tri2_points_aligned) {
					if (p == tri2_p)
						matched = true;
				}
				if (!matched)
					tri2_points_aligned[missing_k] = tri2_p;
			}

			grid.elements.emplace_back(WEDGE);
			Element& e = grid.elements.back();
			e.name_i = name_i;
			if (tri1_faces_out) {
				e.points[0] = tri1_face->points[0];
				e.points[1] = tri1_face->points[1];
				e.points[2] = tri1_face->points[2];
				e.points[3] = tri2_points_aligned[0];
				e.points[4] = tri2_points_aligned[1];
				e.points[5] = tri2_points_aligned[2];
			} else {
				e.points[2] = tri1_face->points[0];
				e.points[1] = tri1_face->points[1];
				e.points[0] = tri1_face->points[2];
				e.points[5] = tri2_points_aligned[0];
				e.points[4] = tri2_points_aligned[1];
				e.points[3] = tri2_points_aligned[2];
			}
		} else if (cell_type == OFHexa) {
			grid.elements.emplace_back(HEXA);
			Element& e = grid.elements.back();
			e.name_i = name_i;
			bool faces_out = (n_owners_per_cell[i] > 0);
			OFFace* first_face = cell_faces[0];
			if (faces_out) {
				e.points[3] = first_face->points[0];
				e.points[2] = first_face->points[1];
				e.points[1] = first_face->points[2];
				e.points[0] = first_face->points[3];
			} else {
				e.points[0] = first_face->points[0];
				e.points[1] = first_face->points[1];
				e.points[2] = first_face->points[2];
				e.points[3] = first_face->points[3];
			}

			for (int j = 1; j < 6; ++j) {
				OFFace* current_face = cell_faces[j];
				for (int k = 0; k < 4; ++k) {
					int p1 = current_face->points[k];
					int p2 = current_face->points[(k+1)%4];

					auto it1 = std::find(first_face->points.begin(),first_face->points.end(),p1);
					bool p1_on_first_face = (it1 != first_face->points.end());

					auto it2 = std::find(first_face->points.begin(),first_face->points.end(),p2);
					bool p2_on_first_face = (it2 != first_face->points.end());

					if (p1_on_first_face != p2_on_first_face) {
						if (p1_on_first_face) {
							for (int l = 0; l < 4; ++l) {
								if (e.points[l] == p1) {
									e.points[l+4] = p2;
									break;
								}
							}
						} else {
							for (int l = 0; l < 4; ++l) {
								if (e.points[l] == p2) {
									e.points[l+4] = p1;
									break;
								}
							}
						}
					}
				}
			}
		} else if (cell_type == OFPoly || cell_type == OFTetraWedge) {
			Point cell_center = calcCellCenter(cell_faces, grid, n_owners_per_cell[i]);
			// need to add new point to grid
			int cell_center_id = grid.points.size();
			grid.points.push_back(cell_center);

			std::vector<Element> new_elements;

			for (int j = 0; j < cell_faces.size(); ++j) {
				bool faces_out = (j < n_owners_per_cell[i]);
				OFFace* current_face = cell_faces[j];

				createElementsFromFaceCenter(*current_face,faces_out,cell_center_id,new_elements);
			}
			grid.elements.insert(grid.elements.end(),new_elements.begin(),new_elements.end());
		}
	}
	int n_volume_elements = grid.elements.size();
	std::vector<Element> negative_elements;
	int negative_volumes = 0;
	int negative_hexas = 0;
	int negative_wedges = 0;
	int negative_tetras = 0;
	int negative_pyramids = 0;
	for (Element& e : grid.elements) {
		if (e.calc_volume(grid) < 0) {
			negative_volumes++;
			negative_elements.push_back(e);
			if (e.type == HEXA)
				negative_hexas++;
			else if (e.type == WEDGE)
				negative_wedges++;
			else if (e.type == TETRA)
				negative_tetras++;
			else if (e.type == PYRAMID)
				negative_pyramids++;
		}
	}
	if (negative_volumes) {
		printf("Negative Volume Elements: %d\n",negative_volumes);
		if (negative_hexas)
			printf("  Hexas: %d\n",negative_hexas);
		if (negative_wedges)
			printf("  Wedges: %d\n",negative_wedges);
		if (negative_tetras)
			printf("  Tetras: %d\n",negative_tetras);
		if (negative_pyramids)
			printf("  Pyramids: %d\n",negative_pyramids);
		Grid g = grid.grid_from_elements(negative_elements);
		toVTK("negative_elements.vtk",g,true);
	}

	for (OFBoundary& boundary : boundaries) {
		int name_i = grid.names.size();
		grid.names.emplace_back(2,boundary.name);
		for (int i = boundary.start_face; i < boundary.start_face+boundary.n_faces; ++i) {
			OFFace& face = faces[i];
			if (face.points.size() < 3) Fatal("1D Boundary Element Found");
			if (face.points.size() == 3 && !face.is_tri_split) {
				grid.elements.emplace_back(TRI);
				Element &e = grid.elements.back();
				e.name_i = name_i;
				for (int j = 0; j < 3; ++j)
					e.points[j] = face.points[j];
			} else if (face.points.size() == 4 && !face.is_tri_split) {
				grid.elements.emplace_back(QUAD);
				Element &e = grid.elements.back();
				e.name_i = name_i;
				for (int j = 0; j < 4; ++j)
					e.points[j] = face.points[j];
			} else {
				assert (face.split_faces.size());
				for (OFFace& new_face : face.split_faces) {
					if (new_face.points.size() == 3) {
						grid.elements.emplace_back(TRI);
						Element &e = grid.elements.back();
						e.name_i = name_i;
						for (int j = 0; j < 3; ++j)
							e.points[j] = new_face.points[j];
					} else if (new_face.points.size() == 4) {
						grid.elements.emplace_back(QUAD);
						Element &e = grid.elements.back();
						e.name_i = name_i;
						for (int j = 0; j < 4; ++j)
							e.points[j] = new_face.points[j];
					}
				}
			}
		}
	}
	int n_boundary_elems = grid.elements.size() - n_volume_elements;
	printf("Created Points: %zu\n",grid.points.size());
	printf("Created Volume Elements: %d\n",n_volume_elements);
	printf("Created Boundary Elements: %d\n",n_boundary_elems);
}
