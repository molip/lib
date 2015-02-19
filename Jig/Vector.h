#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Jig
{
	class Vec3 : public sf::Vector3f
	{
	public:
		Vec3()	{}
		Vec3(const sf::Vector3f& other) : sf::Vector3f(other) {}
		Vec3(float x, float y, float z) : sf::Vector3f(x, y, z) {}

		bool IsZero() const
		{
			return operator ==(Vec3(0, 0, 0));
		}

		bool operator ==(const Vec3& other) const
		{
			float e = 0.00001f;
			return (fabs(x - other.x) < e && fabs(y - other.y) < e && fabs(z - other.z) < e);
		}

		const Vec3& operator *(float f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		// returns cos of angle between Vec3s
		float Dot(const sf::Vector3f& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vec3 Cross(const sf::Vector3f& other) const
		{
			Vec3 result;

			result.x = (y * other.z) - (z * other.y);
			result.y = (z * other.x) - (x * other.z);
			result.z = (x * other.y) - (y * other.x);

			return result;
		}

		float GetLength() const
		{
			return sqrt((x * x) + (y * y) + (z * z));
		}

		float GetLengthSquared() const
		{
			return (x * x) + (y * y) + (z * z);
		}

		void Normalise()
		{
			float m = GetLength();
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

	class Vec2 : public sf::Vector2f
	{
	public:
		Vec2()	{}
		Vec2(const sf::Vector2f& other) : sf::Vector2f(other) {}
		Vec2(float x, float y) : sf::Vector2f(x, y) {}

		bool IsZero() const
		{
			return operator ==(Vec2(0, 0));
		}

		bool operator ==(const Vec2& other) const
		{
			const float e = 0.00001f;
			return fabs(x - other.x) < e && fabs(y - other.y) < e;
		}

		// returns cos of angle
		float Dot(const sf::Vector2f& other) const
		{
			return x * other.x + y * other.y;
		}

		float GetLength() const
		{
			return sqrt(x * x + y * y);
		}

		float GetLengthSquared() const
		{
			return x * x + y * y;
		}

		void Normalise()
		{
			float m = GetLength();
			if (m > 0)
			{
				x /= m; y /= m;
			}
		}
		Vec2 Normalised() const
		{
			Vec2 v = *this;
			v.Normalise();
			return v;
		}
	};
}
