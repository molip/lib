#pragma once

#include "Line2.h"

#include <vector>

namespace Jig
{
	class EdgeMesh;

	class EdgeMeshInternalEdges
	{
	public:
		EdgeMeshInternalEdges(const EdgeMesh& edgeMesh);

		std::vector<Line2> m_lines;

	private:
	};
}