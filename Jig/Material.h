#pragma once

#include "Colour.h"

namespace Jig
{
	class Material
	{
	public:
		Material();
		Material(const Colour& _ambientAndDiffuse) : ambient(_ambientAndDiffuse), diffuse(_ambientAndDiffuse), shininess(0) {}
		void Apply() const;

		Colour ambient, diffuse, specular, emission;
		int shininess;
	};
}