#pragma once

#include <optional>

#include "Vector.h"

namespace Jig
{
	enum class LineAlignment { Inner, Centre, Outer };

	template <typename T>
	std::optional<Vec2T<T>> GetMitreVec(const Vec2T<T>& point, const Vec2T<T>* prev, const Vec2T<T>* next, float thickness)
	{
		Vec2T<T> v1, v2;
		bool valid1{}, valid2{};

		if (prev)
		{
			v1 = point - *prev;
			valid1 = v1.Normalise();
		}

		if (next)
		{
			v2 = *next - point;
			valid2 = v2.Normalise();
		}

		const Vec2T<T> n1(v1.y, -v1.x);
		const Vec2T<T> n2(v2.y, -v2.x);

		Vec2T<T> normal;

		if (!prev)
		{
			if (!valid2)
				return {};

			normal = n2;
		}
		else if (valid1)
		{
			normal = n1;

			if (valid2)
			{
				Vec2T<T> n = n1 + n2;
				if (n.Normalise())
				{
					normal = n;
					thickness /= (float)n1.Dot(normal);
				}
			}
		}
		else
			return {};

		return { normal * Vec2T<T>::Type(thickness) };
	}

	template <typename T>
	std::optional<std::pair<Vec2T<T>, Vec2T<T>>> GetMitrePoints(const Vec2T<T>& point, const Vec2T<T>* prev, const Vec2T<T>* next, float thickness, LineAlignment align)
	{
		auto vec = GetMitreVec(point, prev, next, thickness);
		if (!vec.has_value())
			return {};

		switch (align)
		{
		case LineAlignment::Inner:
			return { { point - vec.value(), point } };
		case LineAlignment::Centre:
			return { { point - vec.value() * (Vec2T<T>::Type)0.5, point + vec.value() * (Vec2T<T>::Type)0.5 } };
		case LineAlignment::Outer:
			return { { point, point + vec.value() } };
		default:
			return {};
		}
	}

}
