#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>
#include <algorithm>
#include <ostream>
#include <istream>

namespace Jig
{
	const double Epsilon = 1e-6f;

	template <typename T>
	class Vec3T : public sf::Vector3<T>
	{
		typedef sf::Vector3<T> Base;
	public:
		using Type = T;

		Vec3T() {}
		Vec3T(T x, T y, T z) : Base(x, y, z) {}
		Vec3T(const Base& other) : Base(other) {}
		template <typename U> Vec3T(const sf::Vector3<U>& rhs) : Base(rhs) {}

		bool IsZero() const
		{
			return operator ==(Base());
		}

		bool operator ==(const Base& other) const
		{
			return (fabs(x - other.x) < (T)Epsilon && fabs(y - other.y) < (T)Epsilon && fabs(z - other.z) < (T)Epsilon);
		}

		const Vec3T<T>& operator *(T f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		// returns cos of angle between Vec3Ts
		T Dot(const Base& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vec3T<T> Cross(const Base& other) const
		{
			Vec3T<T> result;

			result.x = (y * other.z) - (z * other.y);
			result.y = (z * other.x) - (x * other.z);
			result.z = (x * other.y) - (y * other.x);

			return result;
		}

		T GetLength() const
		{
			return sqrt((x * x) + (y * y) + (z * z));
		}

		T GetLengthSquared() const
		{
			return (x * x) + (y * y) + (z * z);
		}

		void Normalise()
		{
			T m = GetLength();
			if (m > 0)
			{
				x /= m; y /= m; z /= m;
			}
		}

		Vec3T<T> Normalised() const
		{
			Vec3T<T> v = *this;
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

	template <typename T>
	class Vec2T : public sf::Vector2<T>
	{
		typedef sf::Vector2<T> Base;
	public:
		using Type = T;
			
		Vec2T() {}
		Vec2T(T x, T y) : Base(x, y) {}
		Vec2T(const Base& other) : Base(other) {}
		template <typename U> Vec2T(const sf::Vector2<U>& rhs) : Base(rhs) {}

		bool IsZero() const
		{
			return operator ==(Base());
		}

		bool operator ==(const Base& other) const
		{
			return fabs(x - other.x) < (T)Epsilon && fabs(y - other.y) < (T)Epsilon;
		}

		// returns cos of angle
		T Dot(const Base& other) const
		{
			return x * other.x + y * other.y;
		}

		// returns sin of angle, going ccw
		// (ie. >0 means other is ccw)
		T DotSine(const Base& other) const
		{
			return ((x * other.y) - (y * other.x));
		}

		T GetAngle(const Vec2T& rhs) const
		{
			assert(IsNormalised() && rhs.IsNormalised());
			return std::copysign(std::acos(std::min(1.0, Dot(rhs))), DotSine(rhs));
		}

		T GetLength() const
		{
			return sqrt(x * x + y * y);
		}

		T GetLengthSquared() const
		{
			return x * x + y * y;
		}

		bool IsNormalised() const
		{
			return std::fabs(GetLengthSquared() - 1.0) < (T)Epsilon;
		}

		bool Normalise()
		{
			T m = GetLength();
			if (m < (T)Epsilon)
				return false;

			x /= m; y /= m;
			return true;
		}

		Vec2T Normalised() const
		{
			Vec2T v = *this;
			assert(v.Normalise());
			return v;
		}
	};

	using Vec3 = Vec3T<double>;
	using Vec2 = Vec2T<double>;

	using Vec3f = Vec3T<float>;
	using Vec2f = Vec2T<float>;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T>& val)
{
	stream << "(" << val.x << "," << val.y << ")";
	return stream;
}

template <typename T>
std::istream& operator>>(std::istream& stream, sf::Vector2<T>& val)
{
	char lparen, comma, rparen;
	T x, y;
	stream >> lparen >> x >> comma >> y >> rparen;

	if (lparen == '(' && comma == ',' && rparen == ')')
		val.x = x, val.y = y;

	return stream;
}
