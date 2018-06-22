#include "EdgeMeshAddFace.h"
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

		auto firstSeg = Line2::MakeFinite(*start.vert, polyline.empty() ? *end.vert : polyline.front());

		if (start.DoesVectorPointInside(firstSeg.GetVector().Normalised()) == external)
			return false;

		if (polyline.empty())
		{
			for (auto& edge : polygon)
				if (edge.vert != start.vert && edge.next->vert != start.vert && edge.vert != end.vert && edge.next->vert != end.vert)
					if (edge.GetLine().Intersect(firstSeg))
						return false;

			return true;
		}

		auto lastSeg = Line2::MakeFinite(polyline.empty() ? *start.vert : polyline.back(), *end.vert);

		for (auto& edge : polygon)
		{
			if (edge.vert != start.vert && edge.next->vert != start.vert)
				if (edge.GetLine().Intersect(firstSeg))
					return false;

			if (edge.vert != end.vert && edge.next->vert != end.vert)
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
		if (!CheckStartAndEnd(polyline, startEdge, endEdge, face.GetEdges(), false))
			return false;

		if (polyline.size() < 2)
			return true; // Already checked.

		return face.Contains(polyline);
	}
}

bool Jig::EdgeMeshAddFace(EdgeMesh& edgeMesh, EdgeMesh::Vert& start, EdgeMesh::Vert& end, const PolyLine& polyline)
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
	{
		Polygon testPoly(polyline);
		for (auto& point : EdgeMesh::OuterPointLoop(*endEdgeOuter, *startEdgeOuter->next->FindSharedOuterEdge()))
			testPoly.push_back(point);

		auto* oldStart = startEdgeOuter;
		auto* oldEnd = endEdgeOuter;
		const bool ccw = !testPoly.IsCW();
		if (ccw)
			std::swap(oldStart, oldEnd);

		auto& face = edgeMesh.AddFace(EdgeMesh::MakeTwinFace(*oldStart, *oldEnd));
		auto& firstTwinned = face.GetEdge();

		auto addPoints = [&](auto& points)
		{
			for (auto& point : points)
			{
				auto& vert = edgeMesh.AddVert(point);
				face.AddAndConnectEdge(&vert, firstTwinned.prev);
			}
		};

		if (ccw)
			addPoints(Kernel::Reverse(polyline));
		else
			addPoints(polyline);

		return true;
	}

	for (auto& face : edgeMesh.GetFaces())
	{
		auto* startEdge = face->FindEdgeWithVert(start);
		auto* endEdge = face->FindEdgeWithVert(end);

		if (startEdge && endEdge && CanSplitFace(*face, polyline, *startEdge, *endEdge))
		{
			EdgeMesh::Edge& newEdge = startEdge->BridgeTo(*endEdge); // startEdge loop now separate. 
			edgeMesh.AddFace(std::make_unique<EdgeMesh::Face>(newEdge));

			auto* edge = &newEdge;
			for (auto& point : polyline)
				edge = &edgeMesh.InsertVert(point, *edge);

			return true;
		}
	}

	return false;
}
