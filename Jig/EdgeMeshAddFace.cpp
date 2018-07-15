#include "EdgeMeshAddFace.h"
#include "EdgeMeshCommand.h"
#include "Geometry.h"
#include "Polygon.h"

using namespace Jig;

namespace
{
	// Returns true if start and end line segment are inside/outside polygon.
	template <typename EdgeLoop>
	bool CheckStartAndEnd(const PolyLine& polyline, const EdgeMesh::Edge& start, const EdgeMesh::Edge& end, const EdgeLoop& polygon, bool external)
	{
		if (polyline.empty())
			if (&start == end.prev || &start == end.next)
				return false; // Degenerate.

		const auto firstSeg = Line2::MakeFinite(*start.vert, polyline.empty() ? *end.vert : polyline.front());

		const auto firstSegVec = firstSeg.GetVector().Normalised();
		const auto prevVec = Vec2(*start.vert - *EdgeLoop::GetPrev(start).vert).Normalised();
		const bool pointsInside = prevVec.GetAngle(firstSegVec) > prevVec.GetAngle(start.GetVec().Normalised());

		if (pointsInside == external)
			return false;

		auto checkVert = [&](const EdgeMesh::Edge& checkEdge, const EdgeMesh::Edge& addingEdge)
		{
			return checkEdge.vert != addingEdge.vert && EdgeLoop::GetNext(checkEdge).vert != addingEdge.vert;
		};
		
		if (polyline.empty())
		{
			for (auto& edge : polygon)
				if (checkVert(edge, start) && checkVert(edge, end))
					if (edge.GetLine().Intersect(firstSeg))
						return false;

			return true;
		}

		auto lastSeg = Line2::MakeFinite(polyline.empty() ? *start.vert : polyline.back(), *end.vert);

		for (auto& edge : polygon)
		{
			if (checkVert(edge, start))
				if (edge.GetLine().Intersect(firstSeg))
					return false;

			if (checkVert(edge, end))
				if (edge.GetLine().Intersect(lastSeg))
					return false;
		}

		return true;
	}

	bool CanAddFace(const PolyLine& polyline, const EdgeMesh::Edge& startEdge, const EdgeMesh::Edge& endEdge)
	{
		KERNEL_ASSERT(!startEdge.twin && !endEdge.twin);

		if (!CheckStartAndEnd(polyline, startEdge, endEdge, EdgeMesh::ConstOuterEdgeLoop(startEdge), true))
			return false;

		if (polyline.size() < 2)
			return true; // Already checked.

		return !Geometry::PolygonIntersectsOrContainsPolyline(EdgeMesh::OuterPointPairLoop(startEdge), polyline.GetPointPairLoop());
	}

	bool CanSplitFace(const EdgeMesh::Face& face, const PolyLine& polyline, const EdgeMesh::Edge& startEdge, const EdgeMesh::Edge& endEdge)
	{
		KERNEL_ASSERT(startEdge.face == &face && endEdge.face == &face);

		if (!CheckStartAndEnd(polyline, startEdge, endEdge, face.GetEdges(), false))
			return false;

		if (polyline.size() < 2)
			return true; // Already checked.

		return face.Contains(polyline);
	}
}

EdgeMeshCommandPtr Jig::EdgeMeshAddFace(EdgeMesh& edgeMesh, EdgeMesh::Vert& start, EdgeMesh::Vert& end, const PolyLine& polyline)
{
	PolyLine polyline2;
	polyline2.push_back(start);
	polyline2.insert(polyline2.end(), polyline.begin(), polyline.end());
	polyline2.push_back(end);

	polyline2.Update();
	if (polyline2.IsSelfIntersecting())
		return false;

	auto* startEdgeOuter = edgeMesh.FindOuterEdgeWithVert(start);
	auto* endEdgeOuter = edgeMesh.FindOuterEdgeWithVert(end);
	if (startEdgeOuter && endEdgeOuter && CanAddFace(polyline, *startEdgeOuter, *endEdgeOuter))
		return std::make_unique<Jig::EdgeMeshCommand::AddOuterFace>(edgeMesh, *startEdgeOuter, *endEdgeOuter, polyline);

	for (auto& face : edgeMesh.GetFaces())
	{
		auto* startEdge = face->FindEdgeWithVert(start);
		auto* endEdge = face->FindEdgeWithVert(end);

		if (startEdge && endEdge && CanSplitFace(*face, polyline, *startEdge, *endEdge))
			return std::make_unique<Jig::EdgeMeshCommand::SplitFace>(edgeMesh, *startEdge, *endEdge, polyline);
	}

	return nullptr;
}
