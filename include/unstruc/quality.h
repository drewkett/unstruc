#ifndef QUALITY_H_C8106684_0039_4CFA_B712_5B9457E0509C
#define QUALITY_H_C8106684_0039_4CFA_B712_5B9457E0509C

#include <vector>

namespace unstruc {
	struct Grid;

	struct MinMax {
		double min, max;

		void update(double value) {
			if (value < min) min = value;
			if (value > max) max = value;
		}

		void update(MinMax other) {
			if (other.min < min) min = other.min;
			if (other.max > max) max = other.max;
		}
	};

	struct MeshQuality {
		MinMax face_angle;
		MinMax dihedral_angle;

		std::vector<int> bad_elements;
	};

	MeshQuality get_mesh_quality(const Grid& grid, double threshold);
}

#endif
