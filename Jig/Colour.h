#pragma once

#include <array>

namespace Jig
{
	class Colour : private std::array < float, 4 >
	{
		using Base = array<float, 4>;
	public:
		Colour() : Base({}) {}
		Colour(float r, float g, float b, float a = 1) : Base({ r, g, b, a }) {}

		operator const float*() const { return data(); }
	};
}