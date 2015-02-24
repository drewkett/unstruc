
#include <cassert>
#include <algorithm>

#include "unstruc.h"
#include "tetmesh.h"

struct OEdge {
	int p1, p2;
	std::vector <int> elements;
	Point min, max;

	OEdge() : p1(-1), p2(-1) {};
	OEdge(int _p1,int _p2) {
		if (_p1 < _p2) {
			p1 = _p1;
			p2 = _p2;
		} else {
			p1 = _p2;
			p2 = _p1;
		}
	};

	OEdge(int _p1,int _p2,int e) {
		if (_p1 < _p2) {
			p1 = _p1;
			p2 = _p2;
		} else {
			p1 = _p2;
			p2 = _p1;
		}
		elements.push_back(e);
	};

	bool operator==(const OEdge& other) const {
		return (p1 == other.p1) && (p2 == other.p2);
	}
	bool operator<(const OEdge& other) const {
		if (p1 == other.p1)
			return p2 < other.p2;
		else
			return p1 < other.p1;
	}

	static bool compare_by_min_x(const OEdge& e1, const OEdge& e2) {
		return e1.min.x < e2.min.x;
	}
};

struct OFace {
	std::vector<int> points;
	std::vector <int> elements;

	Point center;
	Vector normal;
	Point min, max;

	bool operator==(const OFace& other) const {
		return points == other.points;
	}
	bool operator<(const OFace& other) const {
		return points < other.points;
	}

	static bool compare_by_min_x(const OFace& f1, const OFace& f2) {
		return f1.min.x < f2.min.x;
	}
};

std::vector<OEdge> get_edges(Grid& grid) {
	std::vector<OEdge> edges;
	//printf("Create Edges\n");
	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];
		assert (e.type == Shape::Wedge);
		edges.emplace_back(e.points[0],e.points[1],i);
		edges.emplace_back(e.points[1],e.points[2],i);
		edges.emplace_back(e.points[2],e.points[0],i);

		edges.emplace_back(e.points[3],e.points[4],i);
		edges.emplace_back(e.points[4],e.points[5],i);
		edges.emplace_back(e.points[5],e.points[3],i);

		edges.emplace_back(e.points[0],e.points[3],i);
		edges.emplace_back(e.points[1],e.points[4],i);
		edges.emplace_back(e.points[2],e.points[5],i);
	}
	//printf("Sorting Edges\n");
	std::sort(edges.begin(),edges.end());
	int i_edge = 0;
	//printf("Checking Edges for Duplicates\n");
	for (int i = 0; i < edges.size(); ++i) {
		OEdge& edge = edges[i];
		std::vector <int> elements;
		while (i+1 < edges.size() && edge == edges[i+1]) {
			OEdge& edge1 = edges[i+1];
			edge.elements.push_back(edge1.elements[0]);
			++i;
		}
		edges[i_edge] = edge;
		i_edge++;
	}
	edges.resize(i_edge);

	//printf("Setting Edge Properties\n");
	for (OEdge& edge : edges) {
		assert (edge.p1 != -1);
		assert (edge.p2 != -1);
		Point& p1 = grid.points[edge.p1];
		Point& p2 = grid.points[edge.p2];
		edge.min.x = std::min(p1.x,p2.x);
		edge.min.y = std::min(p1.y,p2.y);
		edge.min.z = std::min(p1.z,p2.z);
		edge.max.x = std::max(p1.x,p2.x);
		edge.max.y = std::max(p1.y,p2.y);
		edge.max.z = std::max(p1.z,p2.z);
	}
	return edges;
};

std::vector<OFace> get_faces(Grid& grid) {
	//printf("Creating Faces\n");
	std::vector<OFace> faces;
	//Create Faces from Elements
	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];
		assert (e.type == Shape::Wedge);
		OFace face1;
		face1.elements.push_back(i);
		face1.points.push_back(e.points[0]);
		face1.points.push_back(e.points[1]);
		face1.points.push_back(e.points[2]);
		faces.push_back(face1);

		OFace face2;
		face2.elements.push_back(i);
		face2.points.push_back(e.points[3]);
		face2.points.push_back(e.points[4]);
		face2.points.push_back(e.points[5]);
		faces.push_back(face2);

		OFace face3a;
		face3a.elements.push_back(i);
		face3a.points.push_back(e.points[0]);
		face3a.points.push_back(e.points[1]);
		face3a.points.push_back(e.points[4]);
		face3a.points.push_back(e.points[3]);
		faces.push_back(face3a);

		OFace face3b;
		face3b.elements.push_back(i);
		face3b.points.push_back(e.points[1]);
		face3b.points.push_back(e.points[2]);
		face3b.points.push_back(e.points[5]);
		face3b.points.push_back(e.points[4]);
		faces.push_back(face3b);

		OFace face3c;
		face3c.elements.push_back(i);
		face3c.points.push_back(e.points[2]);
		face3c.points.push_back(e.points[0]);
		face3c.points.push_back(e.points[3]);
		face3c.points.push_back(e.points[5]);
		faces.push_back(face3c);
	}
	//Create face_sort to sort faces after sorting face points, but maintain index to original vector
	//printf("Sorting Faces by Points\n");
	std::vector< std::pair<OFace,int> > face_sort (faces.size());
	for (int i = 0; i < faces.size(); ++i) {
		OFace face = faces[i];
		std::sort(face.points.begin(),face.points.end());
		face_sort[i].first = face;
		face_sort[i].second = i;
	}
	std::sort(face_sort.begin(),face_sort.end());
	std::vector <int> face_indices;
	int i_face = 0;
	//printf("Elimating Duplicate Faces\n");
	for (int i = 0; i < faces.size(); ++i) {
		std::pair <OFace,int>& face_pair = face_sort[i];
		OFace& face = faces[face_pair.second];
		assert(face.elements.size() == 1);
		if (face_pair.first.points.size() == 4) {
			while (i+1 < faces.size() && face_pair.first == face_sort[i+1].first) {
				OFace& face1 = faces[face_sort[i+1].second];
				face.elements.push_back(face1.elements[0]);
				++i;
			}
			assert(face.elements.size() == 2);
		}
		face_indices.push_back(face_pair.second);
		i_face++;
	}
	std::sort(face_indices.begin(),face_indices.end());
	for (int i = 0; i < face_indices.size(); ++i) {
		assert (i <= face_indices[i]);
		faces[i] = faces[face_indices[i]];
	}
	faces.resize(i_face);

	//printf("Setting Face Properties\n");
	for (OFace& face : faces) {
		Point& p0 = grid.points[face.points[0]];
		Point& p1 = grid.points[face.points[1]];
		Point& p2 = grid.points[face.points[2]];
		face.min.x = std::min(std::min(p0.x,p1.x),p2.x);
		face.min.y = std::min(std::min(p0.y,p1.y),p2.y);
		face.min.z = std::min(std::min(p0.z,p1.z),p2.z);
		face.max.x = std::max(std::max(p0.x,p1.x),p2.x);
		face.max.y = std::max(std::max(p0.y,p1.y),p2.y);
		face.max.z = std::max(std::max(p0.z,p1.z),p2.z);
		//double s0 = p0.x + p0.y + p0.z;
		//double s1 = p1.x + p1.y + p1.z;
		//double s2 = p2.x + p2.y + p2.z;
		if (face.points.size() == 3) {
			face.normal = cross(p1-p0, p2-p1);
			face.center.x = (p0.x + p1.x + p2.x)/3;
			face.center.y = (p0.y + p1.y + p2.y)/3;
			face.center.z = (p0.z + p1.z + p2.z)/3;
		} else {
			Point& p3 = grid.points[face.points[3]];
			face.normal = cross(p2-p0, p3-p1);

			double total_length = 0;
			for (int i = 0; i < 4; ++i) {
				Point& p0 = grid.points[face.points[i]];
				Point& p1 = grid.points[face.points[(i+1)%4]];
				Point& p2 = grid.points[face.points[(i+2)%4]];

				Vector v1 = p1 - p0;
				Vector v2 = p2 - p1;
				double length = cross(v1,v2).length();

				face.center.x += (p0.x + p1.x + p2.x)*length/3;
				face.center.y += (p0.y + p1.y + p2.y)*length/3;
				face.center.z += (p0.z + p1.z + p2.z)*length/3;

				total_length += length;
			}
			face.center.x /= total_length;
			face.center.y /= total_length;
			face.center.z /= total_length;
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

std::vector <int> find_negative_volumes(Grid& grid) {
	std::vector <int> negative_volumes;
	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];
		if (e.calc_volume(grid) < 0)
			negative_volumes.push_back(i);
	}
	return negative_volumes;
}

std::vector <int> find_intersections(Grid& grid) {
	std::vector<OEdge> edges = get_edges(grid);
	std::sort(edges.begin(),edges.end(),OEdge::compare_by_min_x);

	std::vector<OFace> faces = get_faces(grid);
	std::sort(faces.begin(),faces.end(),OFace::compare_by_min_x);

	int j_current = 0;
	std::vector <bool> intersected_elements (grid.elements.size(),false);
	for (int i = 0; i < edges.size(); ++i) {
		OEdge& edge = edges[i];
		Point& ep1 = grid.points[edge.p1];
		Point& ep2 = grid.points[edge.p2];
		Vector edge_vector = ep2 - ep1;
		for (int j = j_current; j < faces.size(); ++j) {
			OFace& face = faces[j];
			if (edge.min.x > face.max.x)
				j_current++;
			else
				break;
		}
		for (int j = j_current; j < faces.size(); ++j) {
			OFace& face = faces[j];
			if (face.min.x > edge.max.x) break;
			if (edge.min.x > face.max.x || edge.min.y > face.max.y || edge.min.z > face.max.z) continue;
			if (edge.max.x < face.min.x || edge.max.y < face.min.y || edge.max.z < face.min.z) continue;
			bool same = false;
			for (int p : face.points) {
				if (edge.p1 == p || edge.p2 == p)
					same = true;
				if (ep1 == grid.points[p] || ep2 == grid.points[p])
					same = true;
			}
			if (same) continue;
			double denom = dot(edge_vector,face.normal);
			if (denom == 0) continue;
			double scale = dot(ep1 - face.center,face.normal)/denom;
			double scale2 = dot(ep1 - grid.points[face.points[0]],face.normal)/denom;
			if (face.points.size() == 3) {
				assert (fabs((scale - scale2)/scale) < 1e-5);
			}
			if (-1 <= scale && scale <= 0) {
				Point proj = ep1 - edge_vector*scale;
				bool intersected = true;
				for (int k = 0; k < face.points.size(); ++k) {
					Point& p0 = grid.points[face.points[k]];
					Point& p1 = grid.points[face.points[(k+1)%face.points.size()]];
					Vector v1 = p1 - p0;
					Vector v2 = proj - p1;
					Vector n = cross(v1,v2);
					//double d = dot(n,face.normal)/n.length()/face.normal.length();
					//assert (face.points.size() == 4 || fabs(d) > 0.9);
					if (dot(n,face.normal) <= 0){
						intersected = false;
						break;
					}
				}
				if (intersected) {
					for (int _e : edge.elements)
						intersected_elements[_e] = true;
					for (int _f : face.elements)
						intersected_elements[_f] = true;
				}
			}
		}
	}
	std::vector <int> intersected_list;
	for (int i = 0; i < intersected_elements.size(); ++i) {
		if (intersected_elements[i])
			intersected_list.push_back(i);
	}
	return intersected_list;
}

void write_reduced_file(Grid& grid, std::vector <int> elements, std::string filename) {
	Grid reduced_grid (3);
	reduced_grid.points = grid.points;
	for (int _e : elements)
		reduced_grid.elements.push_back(grid.elements[_e]);
	write_grid(filename,reduced_grid);
}

void print_usage () {
	fprintf(stderr,
"unstruc-offset [-s offset_size] [-n number_of_layers] surface_file output_file\n");
}

void parse_failed (std::string msg) {
	print_usage();
	fprintf(stderr,"\n%s\n",msg.c_str());
	exit(1);
}

Grid volume_from_surfaces (const Grid& surface1, const Grid& surface2) {
	if (surface1.elements.size() != surface2.elements.size())
		Fatal("surfaces don't match");
	int npoints1 = surface1.points.size();

	Grid volume (3);
	volume.points = surface1.points;
	volume.points.insert(volume.points.end(),surface2.points.begin(),surface2.points.end());
	int n_negative = 0;
	for (int i = 0; i < surface1.elements.size(); ++i) {
		const Element& e1 = surface1.elements[i];
		const Element& e2 = surface2.elements[i];
		if (e1.type != e2.type)
			Fatal("elements in surfaces don't match");
		if (e1.type == Shape::Triangle) {
			Element e (Shape::Wedge);
			for (int j = 0; j < 3; ++j) {
				e.points[j] = e1.points[j];
				e.points[j+3] = e2.points[j] + npoints1;
			}
			if (e.calc_volume(volume) < 0) n_negative++;
			volume.elements.push_back(e);
		} else {
			fprintf(stderr,"%s\n",Shape::Info[e1.type].name.c_str());
			NotImplemented("Must pass triangle surfaces");
		}
	}
	if (n_negative == 0) {
	} else if (n_negative == volume.elements.size()) {
#ifndef NDEBUG
		fprintf(stderr,"(unstruc-offset::volume_from_surfaces) Incorrectly oriented. Flipping all volumes.\n");
#endif
		for (Element& e : volume.elements) {
			int temp = e.points[1];
			e.points[1] = e.points[2];
			e.points[2] = temp;
			int temp2 = e.points[4];
			e.points[4] = e.points[5];
			e.points[5] = temp2;
		}
	} else {
		Fatal("Negative Volumes Created");
	}
	return volume;
}

Grid create_offset_surface (const Grid& surface, double offset_size) {
	std::vector< Vector > normals;
	normals.resize(surface.elements.size());

	std::vector< Point > centers;
	centers.resize(surface.elements.size());

	std::vector< std::vector <int> > point_elements;
	point_elements.resize(surface.points.size());

	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e = surface.elements[i];

		const Point& p0 = surface.points[e.points[0]];
		const Point& p1 = surface.points[e.points[1]];
		const Point& p2 = surface.points[e.points[2]];

		Vector v1 = p1 - p0;
		Vector v2 = p2 - p1;
		normals[i] = cross(v1,v2)/6;

		centers[i].x = (p0.x + p1.x + p2.x)/3;
		centers[i].y = (p0.y + p1.y + p2.y)/3;
		centers[i].z = (p0.z + p1.z + p2.z)/3;

		for (int p : e.points)
			point_elements[p].push_back(i);
	}

	std::vector <Vector> point_normals (surface.points.size());
	for (int i = 0; i < surface.points.size(); ++i) {
		const Point& p = surface.points[i];
		std::vector<int>& elements = point_elements[i];

		Vector total_norm;
		for (int j : elements) {
			Vector& n = normals[j];
			Point& c = centers[j];

			Vector v = p - c;
			Vector _n = n/dot(v,v);
			total_norm += _n;
		}
		total_norm /= total_norm.length();
		point_normals[i] = total_norm;
	}

	Grid offset (3);
	offset.elements = surface.elements;
	offset.names = surface.names;
	for (int i = 0; i < surface.points.size(); ++i) {
		const Point& p = surface.points[i];
		Vector& n = point_normals[i];

		Point offset_p = p + n*offset_size;
		offset.points.push_back(offset_p);
	}

	write_grid("offset.vtk",offset);
	Grid offset_volume (3);
	int n_points = surface.points.size();
	offset_volume.points = surface.points;
	offset_volume.points.insert(offset_volume.points.end(),offset.points.begin(),offset.points.end());

	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e1 = surface.elements[i];
		const Element& e2 = offset.elements[i];
		Element e (Shape::Wedge);
		if (offset_size < 0) {
			for (int j = 0; j < 3; ++j) {
				e.points[j] = e1.points[j];
				e.points[j+3] = e2.points[j] + n_points;
			}
		} else {
			for (int j = 0; j < 3; ++j) {
				e.points[2-j] = e1.points[j];
				e.points[5-j] = e2.points[j] + n_points;
			}
		}
		offset_volume.elements.push_back(e);
	}
	write_grid("offset_volume0.vtk",offset_volume);

	for (int i = 1; i < 20; ++i) {
		bool finished = true;
		std::vector <int> negative_volumes = find_negative_volumes(offset_volume);
		printf("%lu Negative Volumes\n",negative_volumes.size());

		if (negative_volumes.size() > 0) {
			finished = false;
			//char filename[50];
			//snprintf(filename,50,"negative_volumes%d.vtk",i);
			//write_reduced_file(volume, negative_volumes, std::string(filename));
		}

		std::vector <int> intersected_elements = find_intersections(offset_volume);
		printf("%lu Intersected Elements\n",intersected_elements.size());

		if (intersected_elements.size() > 0) {
			finished = false;
			//char filename[50];
			//snprintf(filename,50,"intersected_volumes%d.vtk",i);
			//write_reduced_file(volume, intersected_elements, std::string(filename));
		}

		if (finished) break;
		printf("Iteration %d\n",i+1);
		std::vector <bool> poisoned_points (offset_volume.points.size(),false);

		for (int _e : negative_volumes) {
			Element& e = offset_volume.elements[_e];
			for (int p : e.points)
				poisoned_points[p] = true;
		}

		for (int _e : intersected_elements) {
			Element& e = offset_volume.elements[_e];
			for (int p : e.points)
				poisoned_points[p] = true;
		}

		for (Element& e : offset_volume.elements) {
			assert (e.points.size() == 6);
			for (int j = 3; j < 6; ++j) {
				int _p0 = e.points[j-3];
				int _p = e.points[j];
				if (poisoned_points[_p]) {
					if (i < 7) {
						Vector v = offset_volume.points[_p0] - offset_volume.points[_p];
						offset_volume.points[_p] += 0.2*v;
					} else {
						offset_volume.points[_p] = offset_volume.points[_p0];
					}
					poisoned_points[_p] = false;
				}
			}
		}
	}

	std::vector <int> negative_volumes = find_negative_volumes(offset_volume);
	if (negative_volumes.size() > 0) {
		printf("Still %lu Negative Volumes\n",negative_volumes.size());
	}

	std::vector <int> intersected_elements = find_intersections(offset_volume);
	if (intersected_elements.size() > 0) {
		printf("Still %lu Intersected Elements\n",intersected_elements.size());
	}

	write_grid("offset_volume.vtk",offset_volume);

	for (int i = 0; i < n_points; ++i) {
		offset.points[i] = offset_volume.points[i+n_points];
	}
	return offset;
}

int main(int argc, char* argv[]) {
	int argnum = 0;
	std::string inputfile, outputfile;
	double offset_size = 0;
	int nlayers = 1;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			std::string arg (argv[i]);
			if (arg == "-s") {
				++i;
				if (i == argc) parse_failed("Must pass float option to -s");
				offset_size = atof(argv[i]);
			} else if (arg == "-n") {
				++i;
				if (i == argc) parse_failed("Must pass integer to -n");
				nlayers = atoi(argv[i]);
			} else {
				parse_failed("Unknown option passed '"+arg+"'");
			}
		} else {
			argnum++;
			if (argnum == 1) {
				inputfile = std::string(argv[i]);
			} else if (argnum == 2) {
				outputfile = std::string(argv[i]);
			} else {
				parse_failed("Extra argument passed");
			}
		}
	}
	if (argnum != 2)
		parse_failed("Must pass 2 arguments");

	Grid surface = read_grid(inputfile);
	surface.merge_points(0);
	surface.collapse_elements();;
	printf("%zu triangles\n",surface.elements.size());
	printf("%zu points\n",surface.points.size());

	Grid farfield_surface = create_farfield_box(surface);

	Point hole;
	Grid volume;
	if (offset_size != 0) {
		Grid offset_surface = create_offset_surface(surface,offset_size);
		Grid offset_volume = volume_from_surfaces(surface,offset_surface);

		bool found_hole = false;
		for (Element& e : offset_volume.elements) {
			if (!(offset_volume.points[e.points[0]] == offset_volume.points[e.points[3]])) {
				hole = offset_volume.points[e.points[0]];
				found_hole = true;
				break;
			}
		}
		if (!found_hole)
			Fatal("Something went wrong");

		Grid farfield_volume = volgrid_from_surface(offset_surface+farfield_surface,hole,1.03);
		volume = farfield_volume + offset_volume;
	} else {
		Point hole = find_point_inside_surface(surface);
		volume = volgrid_from_surface(surface+farfield_surface,hole,1.03);
	}
	volume += surface;
	volume += farfield_surface;
	volume.merge_points(0);
	write_grid(outputfile,volume);
}
