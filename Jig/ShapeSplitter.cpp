#include "ShapeSplitter.h"

#include "Polygon.h"

#include "Line2.h"
#include "Util.h"

#include <cassert>
#include <map>

using namespace Jig;

namespace
{
	class DeviantAngleFinder
	{
	public:
		DeviantAngleFinder(const EdgeMesh::Edge& edge, bool reverse) : m_edge(edge)
		{
			Vec2 fromPrev = edge.prev->GetVec().Normalised();
			Vec2 fromNext = -edge.GetVec().Normalised();

			if (reverse)
				fromPrev = -fromPrev, fromNext = -fromNext;

			m_normal = (fromPrev + fromNext) / 2.0;
			if (!m_normal.Normalise())
				m_normal = Vec2(-fromPrev.y, fromPrev.x);

			m_maxAngle = m_normal.GetAngle(-fromPrev);
			m_minAngle = m_normal.GetAngle(-fromNext);

			if (m_maxAngle < m_minAngle)
				std::swap(m_maxAngle, m_minAngle);

			assert(m_maxAngle >= 0 && m_minAngle <= 0);
		}

		std::pair<double, bool> GetAngleTo(const Vec2& point) const
		{
			Vec2 vConnect = Vec2(point - *m_edge.vert).Normalised();
			const double angle = m_normal.GetAngle(vConnect);
			return std::make_pair(std::fabs(angle), angle > m_minAngle && angle < m_maxAngle);
		}

	private:
		const EdgeMesh::Edge& m_edge;
		Vec2 m_normal;
		double m_minAngle, m_maxAngle;
	};
}

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

		auto anglesToEdges = GetAnglesToEdges(edge, face, false);
		std::sort(anglesToEdges.begin(), anglesToEdges.end());

		EdgeMesh::Edge* connect = nullptr;
		for (auto& pair : anglesToEdges)
			if (CanConnect(edge, *pair.second))
			{
				connect = pair.second;
				break;
			}

		if (!connect)
			return;

		EdgeMesh::Face& newFace = m_mesh.SplitFace(face, edge, *connect);

		Convexify(face);
		Convexify(newFace);
		return;
	}
}

void ShapeSplitter::AddHole(EdgeMesh::Face& face, EdgeMesh::Face& hole)
{
	assert(face.IsValid());
	assert(hole.IsValid());

	auto results = GetAnglesToHoleEdges(face, hole);
	std::sort(results.begin(), results.end());

	Line2 bridgeLine;
	const EdgeMesh::Edge* bridge0 = nullptr;
	const EdgeMesh::Edge* bridge1 = nullptr;
	for (auto& result : results)
	{
		auto& e0 = *std::get<1>(result);
		auto& e1 = *std::get<2>(result);

		if (CanConnect(e0, e1) && CanConnect(e1, e0))
		{
			if (!bridge0)
			{
				face.Bridge(e0, e1);
				bridge0 = &e0, bridge1 = &e1;
				bridgeLine = Line2::MakeFinite(*e0.vert, *e1.vert);
			}
			else if (bridge0 == &e0 || bridge1 == &e1 || !Line2::MakeFinite(*e0.vert, *e1.vert).Intersect(bridgeLine))
			{
				EdgeMesh::Face& newFace = m_mesh.SplitFace(face, e0, e1);

				Convexify(face);
				Convexify(newFace);

				return;
			}
		}
	}
	assert(false);
}

// Checks e0's siblings for intersection with (e0, e1).
// e1 can be in the same face or not. 
bool ShapeSplitter::CanConnect(const EdgeMesh::Edge& e0, const EdgeMesh::Edge& e1) 
{
	Line2 line = Line2::MakeFinite(*e0.vert, *e1.vert);

	for (EdgeMesh::Edge* e = e0.next; e->next != &e0; e = e->next) // Non-adjacent edges.
	{
		if (e->next != &e1 && e != &e1) // Ignore edges including e1's vert. 
		{
			Line2 edge = Line2::MakeFinite(*e->vert, *e->next->vert);
			if (edge.Intersect(line))
				return false;
		}
	}
	return true;
}

ShapeSplitter::AngleEdgeEdgeCntr ShapeSplitter::GetAnglesToHoleEdges(EdgeMesh::Face& face, EdgeMesh::Face& hole)
{
	AngleEdgeEdgeCntr results;

	for (auto& edge : face.GetEdges())
	{
		auto anglesToEdges = GetAnglesToEdges(edge, hole, true); // Reverse because they're convex. 

		for (auto& pair : anglesToEdges)
		{
			DeviantAngleFinder daf(*pair.second, false);
			auto backAngle = daf.GetAngleTo(*edge.vert);
			if (backAngle.second)
				results.push_back(std::make_tuple(pair.first + backAngle.first, &edge, pair.second));
		}
	}
	return results;
}

// Returns angle between srcEdge's normal and line from srcEdge.vert to each vert in dstFace. 
// srcEdge can be in dstFace or not. 
ShapeSplitter::AngleEdgeCntr ShapeSplitter::GetAnglesToEdges(const EdgeMesh::Edge& srcEdge, EdgeMesh::Face& dstFace, bool reverse)
{
	DeviantAngleFinder daf(srcEdge, reverse);

	// Get candidate verts, best angle first. 
	AngleEdgeCntr vector;
	for (auto& dstEdge : dstFace.GetEdges())
	{
		if (dstEdge.IsConnectedTo(srcEdge))
			continue;

		auto pair = daf.GetAngleTo(*dstEdge.vert);
		if (pair.second)
			vector.push_back(std::make_pair(pair.first, &dstEdge));
	}
	return vector;
}
