
#include <cassert>
#include <algorithm>
#include <climits>

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
#ifndef NDEBUG
	fprintf(stderr,"Create Edges\n");
#endif
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
#ifndef NDEBUG
	fprintf(stderr,"Sorting Faces\n");
#endif
	std::sort(edges.begin(),edges.end());
	int i_edge = 0;
#ifndef NDEBUG
	fprintf(stderr,"Checking Edges for Duplicates\n");
#endif
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

#ifndef NDEBUG
	fprintf(stderr,"Setting Edge Properties\n");
#endif
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
#ifndef NDEBUG
	fprintf(stderr,"Creating Faces\n");
#endif
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
#ifndef NDEBUG
	fprintf(stderr,"Sorting Faces by Points\n");
#endif
	//Create face_sort to sort faces after sorting face points, but maintain index to original vector
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
#ifndef NDEBUG
	fprintf(stderr,"Elimating Duplicate Faces\n");
#endif
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

#ifndef NDEBUG
	fprintf(stderr,"Setting Face Properties\n");
#endif
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
	std::vector <bool> intersected_points (grid.points.size(),false);
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
					intersected_points[edge.p1] = true;
					intersected_points[edge.p2] = true;
					for (int _p : face.points)
						intersected_points[_p] = true;
				}
			}
		}
	}
	std::vector <int> intersected_list;
	for (int i = 0; i < intersected_points.size(); ++i) {
		if (intersected_points[i])
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

struct PointWeight {
	int p;
	double w;
	PointWeight() : p(-1), w(-1) {};
	PointWeight(int p, double w) : p(p), w(w) {};
	bool operator<(const PointWeight& other) { return p < other.p; };
};

struct PointConnection {
	Vector normal;
	std::vector <PointWeight> pointweights;
};

std::vector <PointConnection> calculate_point_connections(const Grid& surface, double offset_size) {
	std::vector< Vector > normals;
	normals.resize(surface.elements.size());

	std::vector< Point > centers;
	centers.resize(surface.elements.size());

	std::vector< std::vector <int> > point_elements (surface.points.size());
	std::vector< std::vector <double> > point_elements_angle (surface.points.size());

	std::vector <PointConnection> point_connections (surface.points.size());

	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e = surface.elements[i];

		if (e.type != Shape::Triangle)
			NotImplemented("(unstruc-offset::calculate_point_connections) Surface must only contain triangles");
		const Point& p0 = surface.points[e.points[0]];
		const Point& p1 = surface.points[e.points[1]];
		const Point& p2 = surface.points[e.points[2]];

		Vector v1 = p1 - p0;
		Vector v2 = p2 - p1;
		normals[i] = cross(v1,v2)/6;

		centers[i].x = (p0.x + p1.x + p2.x)/3;
		centers[i].y = (p0.y + p1.y + p2.y)/3;
		centers[i].z = (p0.z + p1.z + p2.z)/3;

		for (int j = 0; j < e.points.size(); ++j) {
			int _p = e.points[j];
			point_elements[_p].push_back(i);

			int jm = (j - 1 + e.points.size()) % e.points.size();
			int jp = (j + 1) % e.points.size();

			int _pm = e.points[jm];
			int _pp = e.points[jp];

			const Point &pm = surface.points[_pm];
			const Point &p = surface.points[_p];
			const Point &pp = surface.points[_pp];
			Vector vm = pm - p;
			Vector vp = pp - p;
			double angle = fabs(angle_between(vm,vp));
			point_elements_angle[_p].push_back(angle);

			//TODO: Make sure weight by angles is what I want to do for normals smoothing
			PointConnection& pc = point_connections[_p];

			//TODO: Look into avoid connecting points across sharp edges (feature edges)
			pc.pointweights.emplace_back(_pm,angle);
			pc.pointweights.emplace_back(_pp,angle);
		}
	}

	for (int i = 0; i < surface.points.size(); ++i) {
		PointConnection& pc = point_connections[i];
		const Point& p = surface.points[i];
		const std::vector<int>& elements = point_elements[i];
		const std::vector<double>& elements_angle = point_elements_angle[i];

		Vector total_norm;
		double total_angle = 0;
		for (int j = 0; j < elements.size(); ++j) {
			int _e = elements[j];
			double angle = elements_angle[j];

			Vector& n = normals[_e];
			total_norm += angle*n/n.length();
			total_angle += angle;
		}
		total_norm /= total_angle;
		//TODO: Look into adjusting normal size to be larger at nooks and edges
		//TODO: Look into adjusting normal size to provide some smoothing for bumpy surfaces
		pc.normal = total_norm*(offset_size/total_norm.length());

		std::sort(pc.pointweights.begin(),pc.pointweights.end());
		if ((pc.pointweights.size() % 2) != 0) Fatal("Shouldn't be possible");
		double total_weight = 0;
		int new_size = pc.pointweights.size()/2;
		for (int j = 0; j < new_size; ++j) {
			PointWeight& pw0 = pc.pointweights[2*j];
			PointWeight& pw1 = pc.pointweights[2*j+1];
			if (pw0.p != pw1.p) Fatal("Shouldn't be possible");
			double w = pw0.w + pw1.w;
			pc.pointweights[j].p = pw0.p;
			pc.pointweights[j].w = w;
			total_weight += w;
		}
		pc.pointweights.resize(new_size);
		for (PointWeight& pw : pc.pointweights)
			pw.w /= total_weight;
	}
	return point_connections;
}

std::vector <PointConnection> smooth_point_connections(const Grid& surface, const std::vector <PointConnection>& point_connections) {
	std::vector <PointConnection> smoothed_connections (point_connections);

	for (int i = 0; i < point_connections.size(); ++i) {
		const Point& surface_p = surface.points[i];
		const PointConnection& orig_pc = point_connections[i];
		const Vector& orig_normal = orig_pc.normal;
		PointConnection& smoothed_pc = smoothed_connections[i];

		Point smoothed_point;
		for (const PointWeight& pw : orig_pc.pointweights) {
			const Point& p = surface.points[pw.p];
			const Vector& n = point_connections[pw.p].normal;
			double w = pw.w;
			Point offset_p = p+n;
			smoothed_point.x += w*offset_p.x;
			smoothed_point.y += w*offset_p.y;
			smoothed_point.z += w*offset_p.z;
		}
		Vector smoothed_normal = 0.25*(smoothed_point - surface_p) + 0.75*orig_normal;
		smoothed_normal *= orig_normal.length()/smoothed_normal.length();

		smoothed_pc.normal = smoothed_normal;
	}
	return smoothed_connections;
}

Grid offset_surface_with_point_connections(const Grid& surface, const std::vector <PointConnection>& point_connections) {
	Grid offset (3);
	offset.elements = surface.elements;
	offset.names = surface.names;
	offset.points = surface.points;
	for (int i = 0; i < surface.points.size(); ++i) {
		Point& p = offset.points[i];
		const Vector& n = point_connections[i].normal;
		p += n;
	}
	return offset;
}

Grid create_offset_surface (const Grid& surface, double offset_size, const std::string& output_filename) {

	std::vector <PointConnection> point_connections = calculate_point_connections(surface,offset_size);

	Grid presmooth = offset_surface_with_point_connections(surface,point_connections);
	write_grid(output_filename+".presmooth.vtk",presmooth);

	for (int i = 0; i < 15; ++i)
		point_connections = smooth_point_connections(surface,point_connections);

	Grid offset = offset_surface_with_point_connections(surface,point_connections);
	write_grid(output_filename+".offset0.vtk",offset);

	Grid offset_volume (3);
	int n_points = surface.points.size();
	offset_volume.points = surface.points;
	offset_volume.points.insert(offset_volume.points.end(),offset.points.begin(),offset.points.end());

	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e1 = surface.elements[i];
		const Element& e2 = offset.elements[i];
		Element e (Shape::Wedge);
		for (int j = 0; j < 3; ++j) {
			e.points[2-j] = e1.points[j];
			e.points[5-j] = e2.points[j] + n_points;
		}
		offset_volume.elements.push_back(e);
	}

	int last_n_intersected = INT_MAX;
	bool needs_radical_improvement = false;
	for (int i = 1; i < 100; ++i) {
		//TODO Look into calculating and/or smoothing normals per iteration
		bool finished = true;
		std::vector <int> negative_volumes = find_negative_volumes(offset_volume);
		printf("%lu Negative Volumes\n",negative_volumes.size());

		if (negative_volumes.size() > 0) {
			finished = false;
			//char filename[50];
			//snprintf(filename,50,"negative_volumes%d.vtk",i);
			//write_reduced_file(volume, negative_volumes, std::string(filename));
		}

		std::vector <int> intersected_points = find_intersections(offset_volume);
		printf("%lu Intersected Points\n",intersected_points.size());

		if (intersected_points.size() > 0) {
			finished = false;

			if (!needs_radical_improvement) {
				if (intersected_points.size() >= last_n_intersected) {
					needs_radical_improvement = true;
					fprintf(stderr,"Switching to radical measures\n");
				}
				last_n_intersected = intersected_points.size();
			}
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

		for (int _p : intersected_points)
			poisoned_points[_p] = true;

		for (Element& e : offset_volume.elements) {
			assert (e.points.size() == 6);
			for (int j = 3; j < 6; ++j) {
				int _p0 = e.points[j-3];
				int _p = e.points[j];
				if (poisoned_points[_p]) {
					if (needs_radical_improvement) {
						offset_volume.points[_p] = offset_volume.points[_p0];
					} else {
						Vector v = offset_volume.points[_p0] - offset_volume.points[_p];
						offset_volume.points[_p] += 0.1*v;
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

	std::vector <int> intersected_points = find_intersections(offset_volume);
	if (intersected_points.size() > 0) {
		printf("Still %lu Intersected Points\n",intersected_points.size());
	}

	for (int i = 0; i < n_points; ++i) {
		offset.points[i] = offset_volume.points[i+n_points];
	}
	return offset;
}

struct OEdgeElement {
	OEdge edge;
	int element1,element2;
	OEdgeElement() {};
	OEdgeElement(int p1, int p2, int element1, int element2) : edge(p1,p2), element1(element1), element2(element2) {};
	bool operator<(const OEdgeElement& other) { return edge < other.edge; };
};

void verify_complete_surface(const Grid& surface) {
	std::vector <OEdge> edges;
	std::vector < std::vector<OEdgeElement> > edges_per_point (surface.points.size());
	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e = surface.elements[i];
		if (Shape::Info[e.type].dim != 2)
			Fatal("Not a surface. Has non-surface elements");
		for (int j = 0; j < e.points.size(); ++j) {
			int j2 = (j + 1)%e.points.size();
			int p = e.points[j];
			int p2 = e.points[j2];
			edges.emplace_back(p,p2);
			edges_per_point[p].emplace_back(p,p2,i,-1);
			edges_per_point[p2].emplace_back(p,p2,i,-1);
		}
	}
	std::sort(edges.begin(),edges.end());
	for (int i = 0; i < edges.size(); ++i) {
		if (i == edges.size()-1)
			Fatal("Boundary Edge found (1)");
		int i2 = i + 1;
		if (edges[i] != edges[i2])
			Fatal("Boundary Edge found (2)");
		if (i + 2 < edges.size()) {
			int i3 = i + 2;
			if (edges[i2] == edges[i3])
				Fatal("Non-Manifold Edge Found");
		}
		++i;
	}
	for (std::vector <OEdgeElement> edges : edges_per_point) {
		if (edges.size()%2 != 0)
			Fatal("Boundary Edge found (3)");
		std::sort(edges.begin(),edges.end());
		std::list <OEdgeElement> edge_list;
		for (int i = 0; i < edges.size()/2; ++i) {
			const OEdgeElement& ee1 = edges[2*i];
			const OEdgeElement& ee2 = edges[2*i+1];
			if (ee1.edge != ee2.edge)
				Fatal("Boundary Edge found (4)");
			edge_list.emplace_back(ee1.edge.p1,ee2.edge.p2,ee1.element1,ee2.element1);
		}
		std::list <int> surface_list;
		int initial_size = edge_list.size();
		for (int i = 0; i < initial_size; ++i) {
			if (edge_list.size() == 0) break;
			auto it = edge_list.begin();
			while (it != edge_list.end()) {
				OEdgeElement& ee = *it;
				if (surface_list.size() == 0) {
					surface_list.push_back(ee.element1);
					surface_list.push_back(ee.element2);
					it = edge_list.erase(it);
				} else if (ee.element1 == surface_list.front()) {
					surface_list.push_front(ee.element2);
					it = edge_list.erase(it);
				} else if (ee.element2 == surface_list.front()) {
					surface_list.push_front(ee.element1);
					it = edge_list.erase(it);
				} else if (ee.element1 == surface_list.back()) {
					surface_list.push_back(ee.element2);
					it = edge_list.erase(it);
				} else if (ee.element2 == surface_list.back()) {
					surface_list.push_back(ee.element1);
					it = edge_list.erase(it);
				} else {
					it++;
				}
			}
		}
		if (surface_list.front() != surface_list.back())
			Fatal("Boundary Edge Found (5)");
		if (edge_list.size() > 0)
			Fatal("Non-manifold Point Found");
	}
}

int main(int argc, char* argv[]) {
	int argnum = 0;
	std::string input_filename, output_filename;
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
				input_filename = std::string(argv[i]);
			} else if (argnum == 2) {
				output_filename = std::string(argv[i]);
			} else {
				parse_failed("Extra argument passed");
			}
		}
	}
	if (argnum != 2)
		parse_failed("Must pass 2 arguments");

	Grid surface = read_grid(input_filename);
	surface.merge_points(0);
	surface.collapse_elements(false);;
	printf("%zu triangles\n",surface.elements.size());
	printf("%zu points\n",surface.points.size());

	fprintf(stderr,"Verifying surface\n");
	verify_complete_surface(surface);
	Point hole = orient_surface(surface);

	Grid farfield_surface = create_farfield_box(surface);

	Grid volume;
	if (offset_size != 0) {
		Grid offset_surface = create_offset_surface(surface,offset_size,output_filename);
		write_grid(output_filename+".offset.vtk",offset_surface);
		Grid offset_volume = volume_from_surfaces(surface,offset_surface);
		write_grid(output_filename+".offset_volume.vtk",offset_volume);

		//TODO: add layer splitting

		Grid farfield_volume = volgrid_from_surface(offset_surface+farfield_surface,hole,1.03);
		write_grid(output_filename+".farfield_volume.vtk",farfield_volume);
		volume = farfield_volume + offset_volume;
	} else {
		volume = volgrid_from_surface(surface+farfield_surface,hole,1.03);
	}
	volume += surface;
	volume += farfield_surface;
	volume.merge_points(0);
	volume.collapse_elements(false);
	write_grid(output_filename,volume);
}
