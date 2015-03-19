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
		void AddHole(EdgeMesh::Face& face, EdgeMesh::Face& hole);

	private:
		typedef std::vector<std::pair<double, EdgeMesh::Edge*>> AngleEdgeCntr;
		typedef std::vector<std::tuple<double, EdgeMesh::Edge*, EdgeMesh::Edge*>> AngleEdgeEdgeCntr;

		static AngleEdgeCntr GetAnglesToEdges(const EdgeMesh::Edge& edge, EdgeMesh::Face& dstFace, bool reverse);
		static AngleEdgeEdgeCntr GetAnglesToHoleEdges(EdgeMesh::Face& face, EdgeMesh::Face& hole);

		static bool CanConnect(const EdgeMesh::Edge& e0, const EdgeMesh::Edge& e1);

		EdgeMesh& m_mesh;
	};
}
