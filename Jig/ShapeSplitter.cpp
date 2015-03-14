#include "ShapeSplitter.h"

#include "Polygon.h"

#include "Line2.h"
#include "Util.h"

#include <cassert>
#include <map>

using namespace Jig;

ShapeSplitter::ShapeSplitter(EdgeMesh& mesh) : m_mesh(mesh)
{
}

void ShapeSplitter::Convexify(EdgeMesh::Face& face)
{
	assert(face.IsValid());

	if (face.GetEdgeCount() < 4)
		return;

	for (auto& edge : face.GetEdges())
	{
		if (!edge.IsConcave())
			continue;

		EdgeMesh::Edge* connect = ChooseConnectEdge(edge);

		if (!connect)
			return;

		EdgeMesh::Face& newFace = m_mesh.SplitFace(face, edge, *connect);

		Convexify(face);
		Convexify(newFace);
		return;
	}
}
EdgeMesh::Edge* ShapeSplitter::ChooseConnectEdge(const EdgeMesh::Edge& edge) const
{
	assert(edge.IsConcave());

	Vec2 fromPrev = edge.prev->GetVec().Normalised();
	Vec2 fromNext = -edge.GetVec().Normalised();
	Vec2 normal = (fromPrev + fromNext) / 2.0;
	normal.Normalise();

	double angleToPrev = normal.GetAngle(-fromPrev);
	double angleToNext = normal.GetAngle(-fromNext);

	assert(angleToPrev >= 0 && angleToNext <= 0);

	// Get candidate verts, best angle first. 
	std::map<double, EdgeMesh::Edge*> map;
	for (EdgeMesh::Edge* e = edge.next->next; e->next != &edge; e = e->next) // Non-adjacent verts.
	{
		Vec2 vConnect = Vec2(*e->vert - *edge.vert).Normalised();
		const double angle = normal.GetAngle(vConnect);
		if (angle < angleToPrev && angle > angleToNext)
			map.insert(std::make_pair(std::fabs(angle), e));
	}

	for (auto& pair : map)
	{
		EdgeMesh::Edge* connect = pair.second;
		Line2 line = Line2::MakeFinite(*edge.vert, *connect->vert);

		bool hit = false;

		for (EdgeMesh::Edge* e = edge.next; e->next != &edge; e = e->next) // Non-adjacent edges.
		{
			if (e->next != connect && e != connect) // Ignore edges including connect's vert. 
			{
				Line2 edge = Line2::MakeFinite(*e->vert, *e->next->vert);
				if (hit = edge.Intersect(line))
					break;
			}
		}
		if (!hit)
			return connect;
	}
	
	return nullptr;
}
