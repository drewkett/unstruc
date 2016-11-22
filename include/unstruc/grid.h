#ifndef GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE
#define GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE

#include <vector>
#include <string>

#include "point.h"
#include "element.h"

namespace unstruc {
	struct Name {
		size_t dim;
		std::string name;
		Name() : dim(0) {};
		Name(size_t dim,std::string name) : dim(dim), name(name) {};
	};

	struct Grid {
		std::vector <Point> points;
		std::vector <Element> elements;
		std::vector <Name> names;
		size_t dim;

		Grid () : dim(0) {};
		Grid (size_t _dim);
		void merge_points(double tol);
		void delete_inner_faces();
		void collapse_elements(bool split);
		Grid& operator+=(const Grid&);
		Grid operator+(const Grid& other) const { return Grid(*this) += other; };
		void delete_empty_names();
		bool test_point_inside(Point const& p);
		bool check_integrity() const;
		Point get_bounding_min() const;
		Point get_bounding_max() const;
		Grid grid_from_element_index(const std::vector <size_t>& element_index) const;
	};
}

#endif
