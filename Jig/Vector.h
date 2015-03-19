#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>
#include <algorithm>

namespace Jig
{
	const double Epsilon = 1e-6f;

	class Vec3 : public sf::Vector3<double>
	{
		typedef sf::Vector3<double> Base;
	public:
		Vec3()	{}
		Vec3(const Base& other) : Base(other) {}
		Vec3(double x, double y, double z) : Base(x, y, z) {}

		bool IsZero() const
		{
			return operator ==(Vec3(0, 0, 0));
		}

		bool operator ==(const Vec3& other) const
		{
			return (fabs(x - other.x) < Epsilon && fabs(y - other.y) < Epsilon && fabs(z - other.z) < Epsilon);
		}

		const Vec3& operator *(double f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		// returns cos of angle between Vec3s
		double Dot(const Base& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vec3 Cross(const Base& other) const
		{
			Vec3 result;

			result.x = (y * other.z) - (z * other.y);
			result.y = (z * other.x) - (x * other.z);
			result.z = (x * other.y) - (y * other.x);

			return result;
		}

		double GetLength() const
		{
			return sqrt((x * x) + (y * y) + (z * z));
		}

		double GetLengthSquared() const
		{
			return (x * x) + (y * y) + (z * z);
		}

		void Normalise()
		{
			double m = GetLength();
			if (m > 0)
			{
				x /= m; y /= m; z /= m;
			}
		}

		Vec3 Normalised() const
		{
			Vec3 v = *this;
			v.Normalise();
			return v;
		}

		//void glNormal() const
		//{
		//	glNormal3f( x, y, z );
		//}
		//void glVertex() const
		//{
		//	glVertex3f(x, y, z );
		//}
		//void glTranslate() const
		//{
		//	glTranslated( x, y, z );
		//}
		//void glTranslate( double angle ) const
		//{
		//	glRotated( angle, x, y, z );
		//}

	};

	class Vec2 : public sf::Vector2<double>
	{
		typedef sf::Vector2<double> Base;
	public:
		Vec2()	{}
		Vec2(const Base& other) : Base(other) {}
		Vec2(double x, double y) : Base(x, y) {}

		bool IsZero() const
		{
			return operator ==(Vec2(0, 0));
		}

		bool operator ==(const Vec2& other) const
		{
			return fabs(x - other.x) < Epsilon && fabs(y - other.y) < Epsilon;
		}

		// returns cos of angle
		double Dot(const Vec2& other) const
		{
			return x * other.x + y * other.y;
		}

		// returns sin of angle, going ccw
		// (ie. >0 means other is ccw)
		double DotSine(const Vec2& other) const
		{
			return ((x * other.y) - (y * other.x));
		}

		double GetAngle(const Vec2& rhs) const
		{
			assert(IsNormalised() && rhs.IsNormalised());
			return std::copysign(std::acos(std::min(1.0, Dot(rhs))), DotSine(rhs));
		}

		double GetLength() const
		{
			return sqrt(x * x + y * y);
		}

		double GetLengthSquared() const
		{
			return x * x + y * y;
		}

		bool IsNormalised() const
		{
			return std::fabs(GetLengthSquared() - 1.0) < Epsilon;
		}

		bool Normalise()
		{
			double m = GetLength();
			if (m < Epsilon)
				return false;

			x /= m; y /= m;
			return true;
		}

		Vec2 Normalised() const
		{
			Vec2 v = *this;
			assert(v.Normalise());
			return v;
		}
	};
}
