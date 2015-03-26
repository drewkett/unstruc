

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
	};

	MeshQuality get_mesh_quality(const Grid& grid);
}
