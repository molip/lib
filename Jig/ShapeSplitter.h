#pragma once

#include "EdgeMesh.h"
#include <vector>

namespace Jig
{
	class Polygon;

	class ShapeSplitter
	{
	public:
		ShapeSplitter(EdgeMesh& m_mesh);

		void Convexify(EdgeMesh::Face& face);

		EdgeMesh::Edge* ChooseConnectEdge(const EdgeMesh::Edge& edge) const;

	private:

		EdgeMesh& m_mesh;
	};
}
