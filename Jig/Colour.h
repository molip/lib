#pragma once

#include <array>

namespace Jig
{
	class Colour : private std::array < float, 4 >
	{
	public:
		Colour() {}
		Colour(float r, float g, float b, float a = 1) : std::array<float, 4>({ r, g, b, a }) {}

		operator const float*() const { return data(); }
	};
}