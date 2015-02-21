
#include "unstruc.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <list>

struct SurfacePoint {
	Point p;
	std::vector<int> edges;
	std::vector<int> elements;
	SurfacePoint() {};
	SurfacePoint(Point _p) : p(_p) {};
};

struct SurfaceEdge {
	int p1, p2;
	int e1, e2;

	SurfaceEdge() : p1(-1) , p2(-1), e1(-1), e2(-1) {};
	SurfaceEdge(int _p1, int _p2) : p1(_p1) , p2(_p2) {};
	bool operator<(SurfaceEdge other) const {
		if (p1 == other.p1)
			return p2 < other.p2;
		else
			return p1 < other.p1;
	};
};

struct SurfaceElement {
	std::vector<int> points;
	std::vector<int> edges;
	std::vector<int> neighbours;
	Vector normal;
	Point center;
};

struct Surface {
	std::vector<SurfacePoint> points;
	std::vector<SurfaceEdge> edges;
	std::vector<SurfaceElement> elements;
};

std::vector< std::vector<int> > assemble_elements_by_point (Grid& grid) {
	//Assemble convenience vector
	std::vector< std::vector<int> > elements_by_point (grid.points.size());
	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];
		for (int p : e.points)
			elements_by_point[p].push_back(i);
	}
	return elements_by_point;
}

Surface surface_from_grid (Grid& grid) {
	Surface surface;
	for (Point& p : grid.points)
		surface.points.emplace_back(p);
	int i_edge = 0;
	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];
		SurfaceElement s;
		s.points = e.points;

		assert (e.points.size() == 3);
		SurfacePoint& p0 = surface.points[e.points[0]];
		SurfacePoint& p1 = surface.points[e.points[1]];
		SurfacePoint& p2 = surface.points[e.points[2]];

		s.center.x = (p0.p.x + p1.p.x + p2.p.x)/3;
		s.center.y = (p0.p.y + p1.p.y + p2.p.y)/3;
		s.center.z = (p0.p.z + p1.p.z + p2.p.z)/3;

		Vector v1 = p1.p - p0.p;
		Vector v2 = p2.p - p1.p;
		s.normal = cross(v1,v2)/6;

		p0.elements.push_back(i);
		p1.elements.push_back(i);
		p2.elements.push_back(i);

		int n = s.points.size();
		for (int j = 0; j < n; ++j) {
			int j2 = (j + 1) % n;
			SurfaceEdge edge;
			if (s.points[j] < s.points[j2]) {
				edge.p1 = s.points[j];
				edge.p2 = s.points[j2];
			} else {
				edge.p1 = s.points[j2];
				edge.p2 = s.points[j];
			}
			edge.e1 = i;
			surface.edges.push_back(edge);

			s.edges.push_back(i_edge);
			p0.edges.push_back(i_edge);
			p1.edges.push_back(i_edge);
			p2.edges.push_back(i_edge);
			i_edge++;
		}
		s.neighbours.resize(n);

		surface.elements.push_back(s);
	}
	return surface;
}

void combine_edges(Surface& surface) {
	std::vector < std::pair<SurfaceEdge,int> > edges_sorted;
	for (int i = 0; i < surface.edges.size(); ++i) {
		edges_sorted.push_back(std::make_pair(surface.edges[i],i));
	}
	std::sort(edges_sorted.begin(),edges_sorted.end());

	int orig_size = surface.edges.size();
	if (orig_size % 2 != 0) Fatal("Can't be a closed shape");
	int new_size = orig_size/2;

	std::vector <int> new_index (orig_size);
	std::vector <int> neighbours_index (orig_size);
	for (int i_new = 0; i_new < new_size; ++i_new) {
		int i_sort1 = i_new*2;
		int i_sort2 = i_new*2 + 1;

		std::pair <SurfaceEdge,int>& edge_sort1 = edges_sorted[i_sort1];
		std::pair <SurfaceEdge,int>& edge_sort2 = edges_sorted[i_sort2];

		SurfaceEdge& edge1 = edge_sort1.first;
		int i_orig1 = edge_sort1.second;

		SurfaceEdge& edge2 = edge_sort2.first;
		int i_orig2 = edge_sort2.second;

		assert (edge1.p1 == edge2.p1);
		assert (edge1.p2 == edge2.p2);

		new_index[i_orig1] = i_new;
		new_index[i_orig2] = i_new;
		neighbours_index[i_orig1] = edge2.e1;
		neighbours_index[i_orig2] = edge1.e1;

		edge1.e2 = edge2.e1;
		surface.edges[i_new] = edge1;
	}
	surface.edges.resize(new_size);

	for (SurfaceElement& s : surface.elements) {
		for (int j = 0; j < s.edges.size(); ++j) {
			s.neighbours[j] = neighbours_index[s.edges[j]];
			s.edges[j] = new_index[s.edges[j]];
		}
	}
	for (SurfacePoint& p : surface.points)
		for (int j = 0; j < p.edges.size(); ++j)
			p.edges[j] = new_index[p.edges[j]];
}

struct FeatureEdge {
	int iedge;
	bool flipped;

	FeatureEdge(int _iedge, bool _flipped) : iedge(_iedge), flipped(_flipped) {};
};

struct Feature {
	std::list<FeatureEdge> edges;
	std::list<int> points;
};


std::vector<Feature> identify_features(Surface& surface, double feature_angle) {
	std::vector<int> feature_points (surface.points.size(),0);
	std::vector<int> feature_edges;

	for (int i = 0; i < surface.edges.size(); ++i) {
		SurfaceEdge& edge = surface.edges[i];
		SurfaceElement& s1 = surface.elements[edge.e1];
		SurfaceElement& s2 = surface.elements[edge.e2];

		double angle = angle_between(s1.normal,s2.normal);
		if (angle > feature_angle) {
			feature_edges.push_back(i);
			feature_points[edge.p1]++;
			feature_points[edge.p2]++;
		}
	}
	for (int np : feature_points)
		if (np > 2)
			Fatal("Don't know how to handle this situation");

	std::vector<Feature> features;
	for (int ie : feature_edges) {
		SurfaceEdge& e = surface.edges[ie];
		bool found_matching_feature = false;
		for (Feature& f : features) {
			if (e.p1 == f.points.front()) {
				found_matching_feature = true;
				f.points.push_front(e.p2);
				f.edges.emplace_front(ie,true);
			} else if (e.p2 == f.points.front()) {
				found_matching_feature = true;
				f.points.push_front(e.p1);
				f.edges.emplace_front(ie,false);
			} else if (e.p1 == f.points.back()) {
				found_matching_feature = true;
				f.points.push_back(e.p2);
				f.edges.emplace_back(ie,false);
			} else if (e.p2 == f.points.front()) {
				found_matching_feature = true;
				f.points.push_back(e.p1);
				f.edges.emplace_back(ie,true);
			}
			if (found_matching_feature)
				break;
		}
		if (!found_matching_feature) {
			Feature f;
			f.edges.emplace_back(ie,false);
			f.points.push_back(e.p1);
			f.points.push_back(e.p2);
			features.push_back(f);
		}
	}

	for (int i1 = 0; i1 < features.size(); ++i1) {
		Feature& f1 = features[i1];
		if (f1.points.size() == 0) continue;
		for (int i2 = i1+1; i2 < features.size(); ++i2) {
			Feature& f2 = features[i2];
			if (f2.points.size() == 0) continue;
			if (f1.points.front() == f2.points.front()) {
				f1.points.pop_front();

				f1.points.reverse();
				f2.points.insert(f2.points.begin(),f1.points.begin(),f1.points.end());
				f1.points.clear();

				f1.edges.reverse();
				for (FeatureEdge& fe : f1.edges) fe.flipped = !fe.flipped;
				f2.edges.insert(f2.edges.begin(),f1.edges.begin(),f1.edges.end());
				f1.edges.clear();
				break;
			} else if (f1.points.front() == f2.points.back()) {
				f1.points.pop_front();

				f2.points.insert(f2.points.end(),f1.points.begin(),f1.points.end());
				f1.points.clear();

				f2.edges.insert(f2.edges.end(),f1.edges.begin(),f1.edges.end());
				f1.edges.clear();
				break;
			} else if (f1.points.back() == f2.points.back()) {
				f1.points.pop_back();

				f1.points.reverse();
				f2.points.insert(f2.points.end(),f1.points.begin(),f1.points.end());
				f1.points.clear();

				f1.edges.reverse();
				for (FeatureEdge& fe : f1.edges) fe.flipped = !fe.flipped;
				f2.edges.insert(f2.edges.end(),f1.edges.begin(),f1.edges.end());
				f1.edges.clear();
				break;
			} else if (f1.points.back() == f2.points.front()) {
				f1.points.pop_back();

				f2.points.insert(f2.points.begin(),f1.points.begin(),f1.points.end());
				f1.points.clear();

				f2.edges.insert(f2.edges.begin(),f1.edges.begin(),f1.edges.end());
				f1.edges.clear();
				break;
			}
		}
	}
	int _i = 0;
	for (int i = 0; i < features.size(); ++i) {
		if (features[i].points.size() > 0) {
			features[_i] = features[i];
			_i++;
		}
	}
	features.resize(_i);
	//printf("%d Features\n",features.size());
	//for (Feature& f: features) {
	//	printf("Feature\n");
	//	printf("%d",f.points.front());
	//	for (int p : f.points) {
	//		if (p == f.points.front()) continue;
	//		printf("-%d",p);
	//	}
	//	printf("\n");
	//	for (FeatureEdge& e : f.edges) {
	//		SurfaceEdge& edge = surface.edges[e.iedge];
	//		if (e.flipped)
	//			printf("%d-%d ",edge.p2,edge.p1);
	//		else
	//			printf("%d-%d ",edge.p1,edge.p2);
	//	}
	//	printf("\n");
	//}

	return features;
}

static bool SKIP_ENDS = false;
static bool USE_QUADS = true;
Surface split_surface_at_features(Surface& orig_surface, double feature_angle) {
	int n_points = orig_surface.points.size();
	int i_point = n_points;
	int n_edges = orig_surface.edges.size();
	int i_edge = n_edges;

	std::vector<Feature> features = identify_features(orig_surface, feature_angle);

	Surface new_surface;
	new_surface.points = orig_surface.points;
	new_surface.edges = orig_surface.edges;
	new_surface.elements = orig_surface.elements;
	for (SurfacePoint& p : new_surface.points)
		assert (p.elements.size() > 0);

	int new_points = 0;
	int new_edges = 0;
	for (Feature& feature : features) {
		new_edges += feature.edges.size();
		new_points += feature.points.size();
		if (feature.points.front() == feature.points.back()) new_points--;
		if (SKIP_ENDS) {
			new_edges -= 2;
			new_points -= 2;
		}
	}
	new_surface.edges.resize(n_edges + new_edges);
	new_surface.points.resize(n_points + new_points);
	for (Feature& feature : features) {
		std::vector<int> added_points;
		if (SKIP_ENDS && feature.edges.size() == 1) continue;
		for (FeatureEdge& fe : feature.edges) {
			bool first_edge = (&fe == &feature.edges.front());
			bool last_edge = (&fe == &feature.edges.back());
			SurfaceEdge& edge = new_surface.edges[fe.iedge];
			int _p1, _p2;
			if (fe.flipped) {
				_p1 = edge.p2;
				_p2 = edge.p1;
			} else {
				_p1 = edge.p1;
				_p2 = edge.p2;
			}

			SurfacePoint& p1 = new_surface.points[_p1];
			SurfacePoint& p2 = new_surface.points[_p2];

			SurfaceElement& s1 = new_surface.elements[edge.e1];
			SurfaceElement& s2 = new_surface.elements[edge.e2];

			Vector v = p2.p - p1.p;
			Vector v1 = s1.center - p1.p;
			Vector v2 = s2.center - p1.p;
			double test1 = dot(cross(v,v1),s1.normal);
			double test2 = dot(cross(v,v2),s2.normal);
			assert (test1 != 0);
			assert (test2 != 0);
			assert ((test1 > 0) == (test2 < 0));

			int pos_i, neg_i;
			if (test1 > 0) {
				pos_i = edge.e1;
				neg_i = edge.e2;
			} else {
				pos_i = edge.e2;
				neg_i = edge.e1;
			}

			SurfaceElement& pos_s = new_surface.elements[pos_i];
			SurfaceElement& neg_s = new_surface.elements[neg_i];

			if (first_edge && !SKIP_ENDS) {
				new_surface.points[i_point] = p1;

				for (int _s : p1.elements) {
					SurfaceElement& s = new_surface.elements[_s];
					Vector vs = s.center - p1.p;
					double test = dot(cross(v,vs),s.normal);
					test /= s.normal.length();
					test /= cross(v,vs).length();
					if (USE_QUADS && fabs(test) < 0.1) {
						std::vector <int> points;
						for (int j = 0; j < s.points.size(); ++j) {
							points.push_back(s.points[j]);
							if (s.points[j] == _p1)
								points.push_back(i_point);
						}
						s.points = points;
					} else if (test < 0) {
						for (int j = 0; j < s.points.size(); ++j) {
							if (s.points[j] == _p1)
								s.points[j] = i_point;
						}
					}
				}
				added_points.push_back(i_point);
				i_point++;
			}

			if (!(last_edge && SKIP_ENDS)) {
				new_surface.points[i_point] = p2;
				for (int _s : p2.elements) {
					SurfaceElement& s = new_surface.elements[_s];
					Vector vs = s.center - p2.p;
					double test = dot(cross(v,vs),s.normal);
					test /= s.normal.length();
					test /= cross(v,vs).length();
					if (fabs(test) < 0.1 && last_edge) {
						std::vector <int> points;
						for (int j = 0; j < s.points.size(); ++j) {
							points.push_back(s.points[j]);
							if (s.points[j] == _p2)
								points.push_back(i_point);
						}
						s.points = points;
					} else if (test < 0) {
						for (int j = 0; j < s.points.size(); ++j) {
							if (s.points[j] == _p2)
								s.points[j] = i_point;
						}
					}
				}
			}

			bool found_edge_pos = false;
			for (int j = 0; j < pos_s.edges.size(); ++j) {
				if (pos_s.edges[j] == fe.iedge) {
					assert (!found_edge_pos);
					pos_s.neighbours[j] = -1;
					found_edge_pos = true;
					break;
				}
			}
			assert (found_edge_pos);
			
			bool found_edge_neg = false;
			for (int j = 0; j < neg_s.edges.size(); ++j) {
				if (neg_s.edges[j] == fe.iedge) {
					assert (!found_edge_neg);
					neg_s.edges[j] = i_edge;
					neg_s.neighbours[j] = -1;
					found_edge_neg = true;
				}
			}
			assert (found_edge_neg);

			SurfaceEdge& new_edge = new_surface.edges[i_edge];
			if (!(first_edge && SKIP_ENDS))
				new_edge.p1 = i_point-1;
			else
				new_edge.p1 = _p1;
			new_edge.e1 = neg_i;
			new_edge.e2 = -1;

			if (_p2 == feature.points.front()) {
				Fatal();
				assert (_p2 == feature.points.back());
				new_edge.p2 = added_points[0];
			} else {
				if (!(last_edge && SKIP_ENDS)) {
					new_edge.p2 = i_point;
					added_points.push_back(i_point);
					i_point++;
				} else
					new_edge.p2 = _p2;
			}
			i_edge++;
		}
	}

	assert (i_point == new_surface.points.size());
	for (SurfacePoint& p : new_surface.points) {
		p.edges.clear();
		p.elements.clear();
	}
	for (int i = 0; i < new_surface.elements.size(); ++i) {
		SurfaceElement& e = new_surface.elements[i];
		for (int ip : e.points)
			new_surface.points[ip].elements.push_back(i);
	}
	for (int i = 0; i < new_surface.edges.size(); ++i) {
		SurfaceEdge& edge = new_surface.edges[i];
		new_surface.points[edge.p1].edges.push_back(i);
		new_surface.points[edge.p2].edges.push_back(i);
	}
	for (int i = 0; i < new_surface.points.size(); ++i) {
		SurfacePoint& p = new_surface.points[i];
		if (p.elements.size() == 0) {
			printf("%d %d %zu\n",i,n_points,new_surface.points.size());
			Fatal("Point doesn't have any elements");
		}
	}
			
	return new_surface;
}

Grid grid_from_surface(Surface& surface) {
	Grid grid (3);
	for (SurfacePoint& sp: surface.points)
		grid.points.push_back(sp.p);

	for (int i = 0; i < surface.elements.size(); ++i) {
		SurfaceElement& s = surface.elements[i];
		int eltype = 0;
		if (s.points.size() == 3) {
			eltype = TRI;
		} else if (s.points.size() == 4) {
			eltype = QUAD;
		} else {
			Fatal();
		}
		Element e (eltype);
		e.points = s.points;
		grid.elements.push_back(e);
	}
	return grid;
}

Grid volume_from_surfaces(Surface& surface1, Surface& surface2) {
	Grid volume (3);
	for (SurfacePoint& sp: surface1.points)
		volume.points.push_back(sp.p);
	for (SurfacePoint& sp: surface2.points)
		volume.points.push_back(sp.p);

	int n_points1 = surface1.points.size();
	assert (surface1.elements.size() == surface2.elements.size());
	for (int i = 0; i < surface1.elements.size(); ++i) {
		SurfaceElement& s1 = surface1.elements[i];
		SurfaceElement& s2 = surface2.elements[i];
		Element e (WEDGE);
		for (int j = 0; j < 3; ++j) {
			e.points[j] = s1.points[j];
			e.points[j+3] = s2.points[j] + n_points1;
		}
		volume.elements.push_back(e);
	}
	return volume;
}

int main() {
	Grid grid = read_grid("ac.stl");
	grid.merge_points(0);
	grid.collapse_elements();;

	printf("%zu triangles\n",grid.points.size());
	printf("%zu points\n",grid.elements.size());

	Surface surface = surface_from_grid(grid);
	combine_edges(surface);

	Surface featured_surface = split_surface_at_features(surface,120);

	//Get weight point normals
	std::vector <Vector> point_normals (featured_surface.points.size());
	for (int i = 0; i < featured_surface.points.size(); ++i) {
		SurfacePoint& sp = featured_surface.points[i];
		Vector total_norm;
		for (int j : sp.elements) {
			SurfaceElement& s = featured_surface.elements[j];

			Vector v = sp.p - s.center;
			Vector n = s.normal/dot(v,v);
			total_norm += n;
		}
		total_norm /= total_norm.length();
		point_normals[i] = total_norm;
	}

	for (int i = 0; i < featured_surface.points.size(); ++i) {
		SurfacePoint& sp = featured_surface.points[i];
		Vector& n = point_normals[i];

		sp.p -= n*0.0002;
	}

	Grid volume = volume_from_surfaces(surface,featured_surface);
	write_grid("volume.vtk",volume);

	Grid surface_grid = grid_from_surface(surface);
	write_grid("surface.vtk",surface_grid);

	Grid offset_grid = grid_from_surface(featured_surface);
	write_grid("offset.vtk",offset_grid);

	int n_negative_volumes = 0;
	for (int i = 0; i < volume.elements.size(); ++i) {
		Element& e = volume.elements[i];
		if (e.calc_volume(volume) < 0) {
			n_negative_volumes++;
		}
	}
	printf("%d Negative Volumes\n",n_negative_volumes);
}
