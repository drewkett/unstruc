#ifndef POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10
#define POINT_H_17F17439_7F9C_4610_91ED_D673F59EBB10

#include <cmath>

namespace unstruc {
	struct Vector
	{
		double x,y,z;

		inline Vector operator*(double value) const {return Vector(*this) *= value;};
		inline Vector operator/(double value) const {return Vector(*this) /= value;};
		inline Vector& operator*=(double value) {x *= value; y *= value; z *= value; return *this;};
		inline Vector& operator/=(double value) {x /= value; y /= value; z /= value; return *this;};

		inline Vector operator+(const Vector& other) const {return Vector(*this) += other;};
		inline Vector operator-(const Vector& other) const {return Vector(*this) -= other;};
		inline Vector& operator+=(const Vector& other) {x += other.x; y += other.y; z += other.z; return *this;};
		inline Vector& operator-=(const Vector& other) {x -= other.x; y -= other.y; z -= other.z; return *this;};

		inline double dot(const Vector& other) const { return x*other.x + y*other.y + z*other.z; };
		inline double length() const {return sqrt(x*x + y*y + z*z);};
		inline Vector normalized() const {return Vector(*this)/length();};
	};
	template<typename T>
	Vector operator*(T const& value, Vector vec) {
		return vec *= value;
	}

	struct Point
	{
		double x, y, z;

		inline bool operator==(const Point& other) const {return x == other.x && y == other.y && z == other.z;}

		inline Point operator*(double value) const { return Point {x*value,y*value,z*value}; }
		inline Point operator/(double value) const { return Point(*this) /= value; }

		inline Point& operator/=(double value) { x /= value; y /= value; z /= value; return *this; }

		//inline Point operator+(Point other) { return Point (x+other.x,y+other.y,z+other.z); }
		inline Vector operator-(const Point& other) const { return Vector {x-other.x,y-other.y,z-other.z};}

		inline Point operator-(const Vector& other) const { return Point {x-other.x,y-other.y,z-other.z}; }
		inline Point operator+(const Vector& other) const { return Point {x+other.x,y+other.y,z+other.z}; }

		inline void operator-=(const Vector& other) {x -= other.x; y -= other.y; z -= other.z;};
		inline void operator+=(const Vector& other) {x += other.x; y += other.y; z += other.z;};

		inline Point operator+(const Point& other) const { return Point(*this) += other; }

		inline Point& operator+=(const Point& other) {x += other.x; y += other.y; z += other.z; return *this; };

		inline Vector to_vector() const { return Vector { x, y, z }; };
	};

	void dump(Point p);
	bool same(Point& p1, Point& p2, double tol);

	void dump(Vector);
	double angle_between(const Vector&, const Vector&);
	double dot(const Vector&, const Vector&);
	Vector cross(const Vector&, const Vector&);
}

#endif
