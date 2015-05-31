#include "GetVisiblePoints.h"
#include "Debug.h"
#include "Geometry.h"

#include <queue>
#include <functional>

using namespace Jig;

namespace
{
	void AddVisible(const Vec2& point, const Vec2& limit0, const Vec2& limit1, std::set<EdgeMesh::VertPtr>& visible, const EdgeMesh::Edge& enteringEdge)
	{
		for (auto& edge : enteringEdge.face->GetOtherEdges(enteringEdge))
		{
			const Vec2 toStart = Vec2(*edge.vert - point).Normalised();
			if (limit1.GetAngle(toStart) > 0) // Finished.
				break;

			const Vec2 toEnd = Vec2(*edge.next->vert - point).Normalised();
			if (limit0.GetAngle(toEnd) < 0) // Not in range yet.
				continue;

			// Edge is at least partially visible.

			Vec2 newLimit0;
			if (limit0.GetAngle(toStart) >= 0) // Start is visible.
			{
				visible.insert(edge.vert);
				newLimit0 = toStart;
			}
			else
				newLimit0 = limit0;

			if (edge.twin)
			{
				Vec2 newLimit1 = limit1.GetAngle(toEnd) < 0 ? toEnd : limit1;
				AddVisible(point, newLimit0, newLimit1, visible, *edge.twin);
			}
		}
	}

	void AddVisible(const EdgeMesh::Face& face, const Vec2& point, std::set<EdgeMesh::VertPtr>& visible, const EdgeMesh::Edge* enteringEdge)
	{
		for (auto& edge : enteringEdge ? face.GetOtherEdges(*enteringEdge) : face.GetEdges())
		{
			visible.insert(edge.vert);
			
			if (edge.twin)
			{
				Vec2 limit0(*edge.vert - point);
				Vec2 limit1(*edge.next->vert - point);

				if (limit0.Normalise() && limit1.Normalise())
					AddVisible(point, limit0, limit1, visible, *edge.twin);
				else
					AddVisible(*edge.twin->face, point, visible, edge.twin); // Point is on the edge - no need for limits. 
			}
		}
	}
}

std::vector<Vec2> Jig::GetVisiblePoints(const EdgeMesh& mesh, const Vec2 & point)
{
	const EdgeMesh::Face* startFace = mesh.HitTest(point);

	if (!startFace)
		return std::vector<Vec2>();

	std::set<EdgeMesh::VertPtr> visible;

	AddVisible(*startFace, point, visible, nullptr);

	std::vector<Vec2> points;
	points.reserve(visible.size());
	for (auto& p : visible)
		points.push_back(*p);

	return points;
}
