
#include <cassert>
#include <list>
#include <algorithm>
#include <climits>
#include <cfloat>

#include "unstruc.h"
#include "tetmesh.h"

using namespace unstruc;

const static bool use_tangents = true;
const static bool use_sqrt_length = true;
const static bool use_sqrt_angle = true;
const static bool use_normalized_weights = true;
const static bool use_original_offset = false;
const static bool use_smooth_minmax_offset_size = true;

const static bool use_skew_restriction = true;

const static double min_orig_weight = 0.5;
const static double max_normal_skew_angle = 30;
const static double max_skew_angle = 30;
const static double max_relaxed_skew_angle = 45;
const static double tetgen_min_ratio = 1.03;

const static bool use_taubin = false;
namespace taubin {
	const static double kpb = 0.1;
	//const static double taubin_gamma = 0.5;
	const static double gamma = (kpb - 3)/(3*kpb - 5);
	const static double mu = 1/(kpb - 1/gamma);
	const static double n = 20;
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

void write_reduced_file(const Grid& grid, std::vector <int> elements, const std::string& filename) {
	Grid reduced_grid (3);
	reduced_grid.points = grid.points;
	for (int _e : elements)
		reduced_grid.elements.push_back(grid.elements[_e]);
	write_grid(filename,reduced_grid);
}

void write_reduced_file_from_points(const Grid& grid, std::vector <int> points, const std::string& filename) {
	Grid reduced_grid (3);
	reduced_grid.points = grid.points;
	std::sort(points.begin(),points.end());
	for (const Element& e : grid.elements) {
		bool add = false;
		for (int _p : e.points) {
			if (std::binary_search(points.begin(),points.end(),_p)) {
				add = true;
				break;
			}
		}
		if (add)
			reduced_grid.elements.push_back(e);
	}
	write_grid(filename,reduced_grid);
}

Grid volume_from_surfaces (const Grid& surface1, const Grid& surface2) {
	if (surface1.elements.size() != surface2.elements.size())
		fatal("surfaces don't match");
	int npoints1 = surface1.points.size();

	Grid volume (3);
	volume.points = surface1.points;
	volume.points.insert(volume.points.end(),surface2.points.begin(),surface2.points.end());
	int n_negative = 0;
	for (int i = 0; i < surface1.elements.size(); ++i) {
		const Element& e1 = surface1.elements[i];
		const Element& e2 = surface2.elements[i];
		if (e1.type != e2.type)
			fatal("elements in surfaces don't match");
		if (e1.type == Shape::Triangle) {
			Element e (Shape::Wedge);
			for (int j = 0; j < 3; ++j) {
				e.points[2-j] = e1.points[j];
				e.points[5-j] = e2.points[j] + npoints1;
			}
			if (e.calc_volume(volume) < 0) n_negative++;
			volume.elements.push_back(e);
		} else {
			fprintf(stderr,"%s\n",Shape::Info[e1.type].name.c_str());
			not_implemented("Must pass triangle surfaces");
		}
	}
	return volume;
}

struct PointWeight {
	int p;
	double w;
	PointWeight() : p(-1), w(-1) {};
	PointWeight(int p, double w) : p(p), w(w) {};
	bool operator<(const PointWeight& other) const { return p < other.p; };
};

struct PointConnection {
	std::vector <PointWeight> pointweights;
	std::vector <int> elements;
	Vector normal;
	Vector orig_normal;
	double current_adjustment;
	double geometric_severity;
	double orig_min_offset_size;
	double orig_max_offset_size;
	double min_offset_size;
	double max_offset_size;
	double geometric_stretch_factor;
	double max_skew_angle;
	bool convex;
};

struct SmoothingData {
	std::vector <PointConnection> connections;
	std::vector <Vector> element_normals;
};

std::vector<double> normalize(std::vector <double> vec) {
	double total = 0;
	for (double value : vec)
		total += value;
	if (total > 0) {
		for (double& value : vec)
			value /= total;
	}
	return vec;
}


SmoothingData calculate_point_connections(const Grid& surface, double offset_size) {
	std::vector< std::vector <int> > point_elements (surface.points.size());
	std::vector< std::vector <double> > point_elements_angle (surface.points.size());
	std::vector< std::vector <Vector> > point_bisect_vectors (surface.points.size());

	SmoothingData sdata;
	sdata.connections = std::vector <PointConnection> (surface.points.size());
	sdata.element_normals = std::vector <Vector> (surface.elements.size());

	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e = surface.elements[i];

		if (e.type != Shape::Triangle)
			not_implemented("(unstruc-offset::calculate_point_connections) Surface must only contain triangles");
		const Point& p0 = surface.points[e.points[0]];
		const Point& p1 = surface.points[e.points[1]];
		const Point& p2 = surface.points[e.points[2]];

		Vector v1 = p1 - p0;
		Vector v2 = p2 - p1;
		if (cross(v1,v2).length() == 0)
			fatal("Bad Element. Has no normal");
		sdata.element_normals[i] = cross(v1,v2).normalized();

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

			Vector bisect = (vm + vp).normalized();
			point_bisect_vectors[_p].push_back(bisect);

			PointConnection& pc = sdata.connections[_p];

			if (use_tangents) {
				pc.pointweights.push_back( PointWeight(_pm,tan(angle/2.0/180*M_PI)) );
				pc.pointweights.push_back( PointWeight(_pp,tan(angle/2.0/180*M_PI)) );
			} else {
				pc.pointweights.push_back( PointWeight(_pm,angle) );
				pc.pointweights.push_back( PointWeight(_pp,angle) );
			}

			pc.elements.push_back(i);
		}
	}

	for (int i = 0; i < surface.points.size(); ++i) {
		PointConnection& pc = sdata.connections[i];

		pc.max_skew_angle = max_skew_angle;
		pc.current_adjustment = 1;

		const Point& p = surface.points[i];
		const std::vector<int>& elements = point_elements[i];

		const std::vector <double>& angle_factors = normalize(point_elements_angle[i]);
		const std::vector <Vector>& bisect_vectors = point_bisect_vectors[i];

		Vector point_norm { 0, 0, 0 };
		Vector point_bisect { 0, 0, 0 };
		for (int j = 0; j < elements.size(); ++j) {
			int _e = elements[j];
			double fac = angle_factors[j];

			const Vector& n = sdata.element_normals[_e];
			point_norm += fac*n;

			const Vector& bisect = bisect_vectors[j];
			point_bisect += fac*bisect;
		}
		double norm_length = point_norm.length();
		if (point_norm.length() == 0)
			fatal("Point Normal length == 0");

		double convex_test = dot(point_norm.normalized(),point_bisect.normalized());
		pc.convex = convex_test < 0;

		// Correct normals that will cause self intersections
		for (int j = 0; j < 100; ++j) {
			bool all_positive = true;
			for (int _e : elements) {
				const Vector& n = sdata.element_normals[_e];
				double d = dot(point_norm,n);
				if (d <= 0) {
					all_positive = false;
					point_norm -= 2*d*n;
				}
			}
			if (all_positive) break;
		}
		bool bad_vector = false;
		for (int _e : elements) {
			const Vector& n = sdata.element_normals[_e];
			double d = dot(point_norm,n);
			if (d <= 0) {
				fprintf(stderr,"Can't create normal for ");
				dump(p);
				//fatal ("Can't find normal");
				bad_vector = true;
				point_norm *= 0;
				break;
			}
		}
		if (bad_vector) {
			pc.normal = NullVector;
			pc.orig_normal = NullVector;
			continue;
		}

		if (point_norm.length() == 0)
			fatal("Adjusted Point Normal length == 0");
		point_norm = norm_length*point_norm.normalized();

		assert(norm_length < 1 + sqrt(DBL_EPSILON));
		pc.geometric_severity = norm_length;
		if (pc.convex) {
			pc.geometric_stretch_factor = pc.geometric_severity;
			pc.min_offset_size = offset_size*pc.geometric_severity;
			pc.max_offset_size = offset_size*2;
		} else { 
			pc.geometric_stretch_factor = 1/pc.geometric_severity;
			pc.min_offset_size = offset_size;
			pc.max_offset_size = offset_size/pc.geometric_severity*2;
		}
		pc.orig_min_offset_size = pc.min_offset_size;
		pc.orig_max_offset_size = pc.max_offset_size;

		pc.normal = point_norm.normalized()*(offset_size*pc.geometric_stretch_factor);
		pc.orig_normal = pc.normal;

		assert ((pc.pointweights.size() % 2) == 0);

		std::sort(pc.pointweights.begin(),pc.pointweights.end());
		double total_weight = 0;
		int new_size = pc.pointweights.size()/2;
		for (int j = 0; j < new_size; ++j) {
			const PointWeight& pw1 = pc.pointweights[2*j];
			const PointWeight& pw2 = pc.pointweights[2*j+1];

			assert (pw1.p == pw2.p);

			const Point& p1 = surface.points[pw1.p];
			Vector d = p1 - p;
			if (d.length()== 0)
				fatal("Coincedent points found");
			double w;
			if (use_sqrt_angle)
				w = sqrt(pw1.w + pw2.w);
			else
				w = pw1.w + pw2.w;
			if (use_sqrt_length)
				w /= sqrt(d.length());
			else
				w /= d.length();

			total_weight += w;

			pc.pointweights[j].p = pw1.p;
			pc.pointweights[j].w = w;
		}
		pc.pointweights.resize(new_size);
		if (new_size > 0 && total_weight== 0)
			fatal("Weights sum to zero");
		if (use_normalized_weights)
			for (PointWeight& pw : pc.pointweights)
				pw.w /= total_weight;
	}
	if (use_smooth_minmax_offset_size) {
		for (int j = 0; j < 100; ++j) {
			for (int i = 0; i < surface.points.size(); ++i) {
				PointConnection& pc = sdata.connections[i];

				double orig_weight = 0.1 + 0.9*(1-pc.geometric_severity);

				double min_adj = 0;
				double max_adj = 0;

				for (const PointWeight& pw : pc.pointweights) {
					const PointConnection& other_pc = sdata.connections[pw.p];
					double delta_min = other_pc.min_offset_size - pc.orig_min_offset_size;
					min_adj += pw.w * delta_min * (1-orig_weight);

					double delta_max = other_pc.max_offset_size - pc.orig_max_offset_size;
					max_adj += pw.w * delta_max * (1-orig_weight);
				}
				if (min_adj < 0)
					pc.min_offset_size = pc.orig_min_offset_size + min_adj;
				if (max_adj > 0)
					pc.max_offset_size = pc.orig_max_offset_size + max_adj;
			}
		}
	}
	return sdata;
}

void smooth_normals(const Grid& surface, SmoothingData& data) {
	std::vector <PointConnection> smoothed_connections (data.connections);

	for (int i = 0; i < surface.points.size(); ++i) {
		const Point& surface_p = surface.points[i];
		const PointConnection& pc = data.connections[i];
		PointConnection& smoothed_pc = smoothed_connections[i];

		const Vector& curr_normal = pc.normal;
		const Vector& orig_normal = pc.orig_normal;

		if (orig_normal.length() == 0) continue;

		double orig_weight = min_orig_weight + (1-min_orig_weight)*(1 - pc.geometric_severity);

		Vector smoothed_normal (curr_normal);
		for (const PointWeight& pw : pc.pointweights) {
			const Point& p = surface.points[pw.p];
			const Vector& n = data.connections[pw.p].normal;
			double w = pw.w * (1-orig_weight);
			Vector delta = n - curr_normal;
			smoothed_normal.x += w * delta.x;
			smoothed_normal.y += w * delta.y;
			smoothed_normal.z += w * delta.z;
		}

		double perp_length = dot(orig_normal.normalized(),smoothed_normal);

		Vector smoothed_perp = perp_length * orig_normal.normalized();
		Vector smoothed_lateral = smoothed_normal - smoothed_perp;

		double lat_length = smoothed_lateral.length();
		perp_length = smoothed_perp.length();

		const double max_normal_skew_factor = tan(max_normal_skew_angle*pc.geometric_severity/180.0*M_PI);
		if (use_skew_restriction && lat_length > 0 && lat_length > max_normal_skew_factor*perp_length)
			smoothed_lateral *= max_normal_skew_factor*perp_length/lat_length;

		smoothed_normal = smoothed_lateral + smoothed_perp;

		smoothed_pc.normal = smoothed_normal.normalized()*orig_normal.length();
		for (int _e : pc.elements) {
			const Vector& n = data.element_normals[_e];
			if (dot(smoothed_pc.normal,n) <= 0) {
				// Use old normal if self intersections created
				smoothed_pc.normal = pc.normal;
				break;
			}
		}
	}
	data.connections = smoothed_connections;
}

void smooth_point_connections(const Grid& surface, SmoothingData& data) {
	std::vector <PointConnection> smoothed_connections (data.connections);

	for (int i = 0; i < surface.points.size(); ++i) {
		const Point& surface_p = surface.points[i];
		const PointConnection& pc = data.connections[i];
		PointConnection& smoothed_pc = smoothed_connections[i];

		const Vector& curr_normal = pc.normal;
		const Vector& orig_normal = pc.orig_normal;
		const double max_normal_skew_factor = tan(pc.max_skew_angle*pc.geometric_severity/180.0*M_PI);

		if (orig_normal.length() == 0) continue;

		double orig_weight = min_orig_weight + (1-min_orig_weight)*(1 - pc.geometric_severity);
		Point orig_p;
		if (use_original_offset)
			orig_p = surface_p + orig_normal;
		else
			orig_p = surface_p + curr_normal;

		Point smoothed_point (orig_p);
		for (const PointWeight& pw : pc.pointweights) {
			const Point& p = surface.points[pw.p];
			const Vector& n = data.connections[pw.p].normal;
			double w = pw.w * (1-orig_weight);
			Point offset_p = p+n;
			Vector delta = offset_p - orig_p;
			smoothed_point.x += w * delta.x;
			smoothed_point.y += w * delta.y;
			smoothed_point.z += w * delta.z;
		}
		Vector smoothed_normal = smoothed_point - surface_p;

		assert (orig_normal.length() > 0);

		double perp_length = dot(orig_normal.normalized(),smoothed_normal);

		Vector smoothed_perp = perp_length * orig_normal.normalized();
		Vector smoothed_lateral = smoothed_normal - smoothed_perp;

		if (perp_length < pc.min_offset_size*pc.current_adjustment) smoothed_perp *= pc.min_offset_size*pc.current_adjustment/perp_length;
		else if (perp_length > pc.max_offset_size) smoothed_perp *= pc.max_offset_size/perp_length;

		double lat_length = smoothed_lateral.length();
		perp_length = smoothed_perp.length();
		if (use_skew_restriction && lat_length > 0 && lat_length > max_normal_skew_factor*perp_length)
			smoothed_lateral *= max_normal_skew_factor*perp_length/lat_length;
		smoothed_pc.normal = smoothed_lateral + smoothed_perp;

		// Check for creation of self intersection elements
		for (int _e : pc.elements) {
			const Vector& n = data.element_normals[_e];
			if (dot(smoothed_pc.normal,n) <= 0) {
				// Use old normal if self intersections created
				smoothed_pc.normal = pc.normal;
				break;
			}
		}
	}
	data.connections = smoothed_connections;
}

void smooth_point_connections_taubin(const Grid& surface, SmoothingData& data, double gamma) {
	std::vector <PointConnection> smoothed_connections (data.connections);

	for (int i = 0; i < surface.points.size(); ++i) {
		const Point& surface_p = surface.points[i];
		const PointConnection& pc = data.connections[i];
		PointConnection& smoothed_pc = smoothed_connections[i];

		const Vector& curr_normal = pc.normal;
		const Vector& orig_normal = pc.orig_normal*pc.current_adjustment;
		const double max_normal_skew_factor = tan(pc.max_skew_angle*pc.geometric_severity/180.0*M_PI);

		if (orig_normal.length() == 0) continue;

		Point orig_p;
		if (use_original_offset)
			orig_p = surface_p + orig_normal;
		else
			orig_p = surface_p + curr_normal;

		Point smoothed_point (orig_p);
		for (const PointWeight& pw : pc.pointweights) {
			const Point& p = surface.points[pw.p];
			const Vector& n = data.connections[pw.p].normal;
			double w = pw.w * gamma;
			Point offset_p = p + n;
			Vector delta = offset_p - orig_p;
			smoothed_point.x += w * delta.x;
			smoothed_point.y += w * delta.y;
			smoothed_point.z += w * delta.z;
		}
		Vector smoothed_normal = smoothed_point - surface_p;
		if (gamma > 0) {
			smoothed_pc.normal = smoothed_normal;
		} else {
			double perp_length = dot(orig_normal.normalized(),smoothed_normal);
			assert (perp_length > 0);

			Vector smoothed_perp = perp_length * orig_normal.normalized();
			Vector smoothed_lateral = smoothed_normal - smoothed_perp;

			if (perp_length < pc.min_offset_size) smoothed_perp *= pc.min_offset_size/perp_length;
			else if (perp_length > pc.max_offset_size) smoothed_perp *= pc.max_offset_size/perp_length;

			double lat_length = smoothed_lateral.length();
			perp_length = smoothed_perp.length();
			if (use_skew_restriction && lat_length > 0 && lat_length > max_normal_skew_factor*perp_length)
				smoothed_lateral *= max_normal_skew_factor*perp_length/lat_length;
			smoothed_pc.normal = smoothed_lateral + smoothed_perp;

			// Check for creation of self intersection elements
			for (int _e : pc.elements) {
				const Vector& n = data.element_normals[_e];
				if (dot(smoothed_pc.normal,n) <= 0) {
					// Use old normal if self intersections created
					smoothed_pc.normal = pc.normal;
					break;
				}
			}
		}
	}
	data.connections = smoothed_connections;
}

Grid offset_surface_with_point_connections(const Grid& surface, std::vector <PointConnection>& point_connections) {
	Grid offset (3);
	offset.elements = surface.elements;
	offset.names = surface.names;
	offset.points = surface.points;
	for (int i = 0; i < surface.points.size(); ++i) {
		Point& p = offset.points[i];
		PointConnection& pc = point_connections[i];
		const Vector& n = pc.normal;
		p += n;
	}
	return offset;
}

void write_grid_with_data (std::string filename, const Grid& surface, const SmoothingData& smoothing_data) {
	std::vector <Vector> orig_normals, normals;
	std::vector <double> geometric_severity, min_offset_size, max_offset_size;
	std::vector <double> orig_min_offset_size, orig_max_offset_size;

	int n_points = surface.points.size();

	orig_normals.reserve(n_points);
	normals.reserve(n_points);
	geometric_severity.reserve(n_points);
	min_offset_size.reserve(n_points);
	max_offset_size.reserve(n_points);
	orig_min_offset_size.reserve(n_points);
	orig_max_offset_size.reserve(n_points);

	for (const PointConnection& pc : smoothing_data.connections) {
		orig_normals.push_back(pc.orig_normal);
		normals.push_back(pc.normal);
		geometric_severity.push_back(pc.geometric_severity);
		min_offset_size.push_back(pc.min_offset_size);
		max_offset_size.push_back(pc.max_offset_size);
		orig_min_offset_size.push_back(pc.orig_min_offset_size);
		orig_max_offset_size.push_back(pc.orig_max_offset_size);
	}

	write_grid(filename,surface);
	vtk_write_point_data_header(filename,surface);
	vtk_write_data(filename,"orig_normals",orig_normals);
	vtk_write_data(filename,"normals",normals);
	vtk_write_data(filename,"geometric_severity",geometric_severity);
	vtk_write_data(filename,"orig_min_offset_size",orig_min_offset_size);
	vtk_write_data(filename,"orig_max_offset_size",orig_max_offset_size);
	vtk_write_data(filename,"min_offset_size",min_offset_size);
	vtk_write_data(filename,"max_offset_size",max_offset_size);
}

void fix_offset_skew ( const Grid& surface, Grid& offset ) {
	std::vector <Vector> surface_normals (surface.elements.size());
	for (int _e = 0; _e < surface.elements.size(); ++_e) {
		const Element& e = surface.elements[_e];
		if (e.type != Shape::Triangle) fatal();

		const Point& p0 = surface.points[e.points[0]];
		const Point& p1 = surface.points[e.points[1]];
		const Point& p2 = surface.points[e.points[2]];

		surface_normals[_e] = cross(p1 - p0, p2 - p1).normalized();
	}
	int total_fixed = 0;
	int fixed = 1;
	while (fixed > 0) {
		fixed = 0;
		for (int _e = 0; _e < surface.elements.size(); ++_e) {
			const Vector& surface_normal = surface_normals[_e];

			const Element& e = offset.elements[_e];
			const Point& p0 = offset.points[e.points[0]];
			const Point& p1 = offset.points[e.points[1]];
			const Point& p2 = offset.points[e.points[2]];

			Vector offset_normal = cross(p1 - p0, p2 - p1).normalized();

			if (dot(offset_normal,surface_normal) < 0.5) {
				const Element& se = surface.elements[_e];
				const Point& sp0 = surface.points[e.points[0]];
				const Point& sp1 = surface.points[e.points[1]];
				const Point& sp2 = surface.points[e.points[2]];

				Point surface_center = (sp0 + sp1 + sp2)/3;
				
				double min_off = DBL_MAX;
				for (int i = 0; i < 3; ++i) {
					const Point& sp = surface.points[e.points[i]];
					const Point& op = offset.points[e.points[i]];
					double off;
					if (sp == op) {
						off = 0;
					} else {
						Vector n = (op - sp);
						double off = dot(n,surface_normal);
					}
					if (off < min_off)
						min_off = off;
				}
				if (min_off < 0) fatal("Shouldn't be possible");
				for (int i = 0; i < 3; ++i) {
					const Point& sp = surface.points[e.points[i]];
					const Point& op = offset.points[e.points[i]];
					Vector n = (op - sp);
					double off = dot(n,surface_normal);
					if (off > 1.5*min_off) {
						fixed++;
						dump(offset.elements[_e],offset);
						if (min_off == 0) {
							offset.points[e.points[i]] = sp;
						} else {
							double ratio = min_off/off + DBL_EPSILON;
							offset.points[e.points[i]] = sp + ratio*n;
						}
					}
				}
			}
		}
		if (fixed > 0)
			printf("%d Fixed Points due to skew\n",fixed);
		total_fixed += fixed;
	}
	if (total_fixed > 0)
		printf("%d Total Fixed Points due to skew\n",total_fixed);
}

Grid create_offset_surface (const Grid& surface, double offset_size, std::string filename) {

	SmoothingData smoothing_data = calculate_point_connections(surface,offset_size);

	Grid presmooth = offset_surface_with_point_connections(surface,smoothing_data.connections);
	write_grid(filename+".presmooth.stl",presmooth);

	for (int i = 0; i < 10; ++i)
		smooth_normals(surface,smoothing_data);
	write_grid_with_data(filename+".data.vtk",surface,smoothing_data);
	for (PointConnection& pc : smoothing_data.connections)
		pc.orig_normal = pc.normal;

	if (use_taubin) {
		for (int i = 0; i < taubin::n; ++i) {
			smooth_point_connections_taubin(surface,smoothing_data,taubin::gamma);
			smooth_point_connections_taubin(surface,smoothing_data,taubin::mu);
		}
	} else {
		for (int i = 0; i < 100; ++i)
			smooth_point_connections(surface,smoothing_data);
	}

	write_grid_with_data(filename+".data2.vtk",surface,smoothing_data);
	Grid offset = offset_surface_with_point_connections(surface,smoothing_data.connections);
	write_grid(filename+".smoothed.stl",offset);

	Grid offset_volume = volume_from_surfaces(surface,offset);

	int n_surface_points = surface.points.size();
	int last_n_intersected = INT_MAX;
	int last_n_negative = INT_MAX;
	bool needs_radical_improvement = false;
	int failed_steps = 0;
	bool successful;
	std::vector <int> intersected_elements;
	int n_full_iterations = 0;
	for (int i = 1; i < 1000; ++i) {
		if (n_full_iterations > 100) break;

		successful = true;
		std::vector <int> negative_volumes = find_negative_volumes(offset_volume);

		if (negative_volumes.size() > 0) {
			fprintf(stderr,"%lu Negative Volumes\n",negative_volumes.size());
			successful = false;
			//if (i == 1)
			//	write_reduced_file(offset_volume,negative_volumes,filename+".negative_elements.0.vtk");
		}

		Intersections intersections;
		if (intersected_elements.size() and !failed_steps) {
			fprintf(stderr,"Checking for Intersections (Subselection)\n");
			Grid intersected_volume = offset_volume.extract_from_element_index(intersected_elements);
			intersections = Intersections::find(intersected_volume);
			if (intersections.elements.size() == 0) {
				//fprintf(stderr,"%lu Intersected Elements\n",intersections.elements.size());
				fprintf(stderr,"Checking for Intersections\n");
				intersections = Intersections::find(offset_volume);
				n_full_iterations++;
			}
		} else {
			fprintf(stderr,"Checking for Intersections\n");
			intersections = Intersections::find(offset_volume);
			n_full_iterations++;
		}
		intersected_elements = intersections.elements;

		if (intersections.elements.size() > 0) {
			fprintf(stderr,"%lu Intersected Elements\n",intersections.elements.size());
			successful = false;
			//if (i == 1)
			//	write_reduced_file(offset_volume,intersections.elements,filename+".intersected_elements.0.vtk");
		}

		if (successful) break;

		if (!needs_radical_improvement) {
			if (intersections.elements.size() >= last_n_intersected && negative_volumes.size() >= last_n_negative) {
				failed_steps++;
				fprintf(stderr,"Failed iteration\n");
				if (failed_steps > 1) {
					needs_radical_improvement = true;
					fprintf(stderr,"Switching to radical measures\n");
				}
			} else
				failed_steps = 0;
			last_n_intersected = intersections.elements.size();
			last_n_negative = negative_volumes.size();
		}

		printf("Iteration %d\n",i);
		std::vector <bool> poisoned_points (offset_volume.points.size(),false);

		for (int _e : negative_volumes) {
			Element& e = offset_volume.elements[_e];
			for (int p : e.points)
				poisoned_points[p] = true;
		}

		for (int _p : intersections.points)
			poisoned_points[_p] = true;

		for (Element& e : offset_volume.elements) {
			assert (e.points.size() == 6);
			for (int j = 3; j < 6; ++j) {
				int _p0 = e.points[j-3];
				int _p = e.points[j];
				if (poisoned_points[_p]) {
					PointConnection& pc = smoothing_data.connections[_p-n_surface_points];
					if (needs_radical_improvement) {
						pc.current_adjustment = 0;
						pc.orig_normal *= 0;
						pc.normal *= 0;
					} else {
						pc.current_adjustment *= 0.9;
						if (pc.current_adjustment < 0.6)
							pc.current_adjustment = 0.6;
						pc.max_skew_angle = max_relaxed_skew_angle;
					}
					poisoned_points[_p] = false;
				}
			}
		}
		for (int i = 0; i < 20; ++i) {
			smooth_point_connections(surface,smoothing_data);
		}

		Grid offset = offset_surface_with_point_connections(surface,smoothing_data.connections);

		for (int j = 0; j < n_surface_points; ++j)
			offset_volume.points[j+n_surface_points] = offset.points[j];
	}

	if (!successful) {
		std::vector <int> negative_volumes = find_negative_volumes(offset_volume);
		if (negative_volumes.size() > 0) {
			printf("%lu Negative Volumes\n",negative_volumes.size());
			fatal("Still Negative Volumes");
		}

		fprintf(stderr,"Checking for Intersections");
		Intersections intersections = Intersections::find(offset_volume);
		if (intersections.elements.size() > 0) {
			fprintf(stderr,"%lu Intersected Points\n",intersections.elements.size());
			fatal("Still Intersections");
		}
	}

	fix_offset_skew(surface,offset);
	for (int i = 0; i < n_surface_points; ++i) {
		offset.points[i] = offset_volume.points[i+n_surface_points];
	}
	return offset;
}

void print_usage () {
	fprintf(stderr,
"unstruc-offset [options] surface_file output_file\n"
"-g growth_rate		  Set target growth rate between layers (Default = 1.5)\n"
"-n number_of_layers  Set target number of layers to add (Default = 1)\n"
"-s offset_size       Set offset size for first layer. No layers generated if option not set (Default = 0)\n"
"-h                   Print Usage\n");
}

int parse_failed (std::string msg) {
	print_usage();
	fprintf(stderr,"\n%s\n",msg.c_str());
	return 1;
}

int main(int argc, char* argv[]) {
	int argnum = 0;
	std::string input_filename, output_filename;
	double offset_size = 0;
	double growth_rate = 1.5;
	int nlayers = 1;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			std::string arg (argv[i]);
			if (arg == "-s") {
				++i;
				if (i == argc) return parse_failed("Must pass float option to -s");
				offset_size = atof(argv[i]);
			} else if (arg == "-n") {
				++i;
				if (i == argc) return parse_failed("Must pass integer to -n");
				nlayers = atoi(argv[i]);
			} else if (arg == "-g") {
				++i;
				if (i == argc) return parse_failed("Must pass float to -n");
				growth_rate = atof(argv[i]);
			} else if (arg == "-h") {
				print_usage();
				return 0;
			} else {
				return parse_failed("Unknown option passed '"+arg+"'");
			}
		} else {
			argnum++;
			if (argnum == 1) {
				input_filename = std::string(argv[i]);
			} else if (argnum == 2) {
				output_filename = std::string(argv[i]);
			} else {
				return parse_failed("Extra argument passed");
			}
		}
	}
	if (argnum != 2)
		return parse_failed("Must pass 2 arguments");

	Grid surface = read_grid(input_filename);
	surface.merge_points(0);
	surface.collapse_elements(false);
	printf("%zu triangles\n",surface.elements.size());
	printf("%zu points\n",surface.points.size());

	fprintf(stderr,"Verifying surface\n");
	std::vector <Point> holes = tetmesh::orient_surfaces(surface);

	write_grid(output_filename+".surface.su2",surface);

	Grid volume;
	if (offset_size != 0) {
		write_grid(output_filename+".0.offset.stl",surface);

		Grid offset_volume (3);
		Grid offset_surface (3);
		Grid last_offset_surface (surface);
		double current_offset_size = offset_size;
		for (int i = 0; i < nlayers; ++i) {
			char c_filename[50];
			snprintf(c_filename,50,"%s.%d",output_filename.c_str(),i+1);
			std::string filename (c_filename);

			offset_surface = create_offset_surface(last_offset_surface,current_offset_size,filename);

			write_grid(filename+".offset.stl",offset_surface);
			const Grid& input_surface = offset_surface;

			offset_volume += volume_from_surfaces(last_offset_surface,offset_surface);

			current_offset_size *= growth_rate;
			last_offset_surface = offset_surface;
		}
		offset_volume.merge_points(0);
		offset_volume.collapse_elements(false);
		write_grid(output_filename+".offset_volume.vtk",offset_volume);

		Grid farfield_surface = tetmesh::create_farfield_box(offset_surface);
		Grid farfield_volume = tetmesh::volgrid_from_surface(offset_surface+farfield_surface,holes,tetgen_min_ratio);
		write_grid(output_filename+".farfield_volume.vtk",farfield_volume);
		volume = farfield_volume + offset_volume + farfield_surface + surface;
	} else {
		Grid farfield_surface = tetmesh::create_farfield_box(surface);
		volume = tetmesh::volgrid_from_surface(surface+farfield_surface,holes,tetgen_min_ratio);
		volume += farfield_surface + surface;
	}
	volume.merge_points(0);
	volume.collapse_elements(false);
	write_grid(output_filename,volume);
}
