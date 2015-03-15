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
		bool CanConnect(const EdgeMesh::Edge& e0, const EdgeMesh::Edge& e1) const;

		EdgeMesh& m_mesh;
	};
}
