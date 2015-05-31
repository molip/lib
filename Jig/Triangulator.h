#pragma once

#include "EdgeMesh.h"

#include <vector>

namespace Jig
{
	class Polygon;

	class Triangulator
	{
	public:
		Triangulator(const Polygon& poly);
		~Triangulator();

		void AddHole(const Polygon& poly);

		EdgeMesh Go();
	private:
		const Polygon& m_poly;
		std::vector<const Polygon*> m_holes;

	};
}