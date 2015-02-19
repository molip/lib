#pragma once

#include "Colour.h"

namespace Jig
{
	class Material
	{
	public:
		Material();
		void Apply() const;

		Colour ambient, diffuse, specular, emission;
		int shininess;
	};
}