#include "PathFinder.h"
#include "Geometry.h"

#include <queue>
#include <functional>

using namespace Jig;

PathFinder::PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint) : m_startPoint(startPoint), m_endPoint(endPoint)
{
	m_startFace = mesh.HitTest(startPoint);
	m_endFace = mesh.HitTest(endPoint);
}

PathFinder::~PathFinder()
{
}

double PathFinder::GetG(const Vec2& point)
{
	return 0;
}

double PathFinder::GetPriority(const EdgeMesh::Face& face)
{
	double h = 0;
	Vec2 point = Geometry::GetClosestPoint(face.GetLineLoop(), m_endPoint, &h);
	return h;
}

PathFinder::FacePath PathFinder::Find()
{
	if (!m_startFace || !m_endFace)
		return FacePath();

	if (m_startFace == m_endFace)
		return FacePath{ m_startFace, m_startFace };
	
	typedef std::pair<double, const EdgeMesh::Face*> T;
	std::priority_queue<T, std::vector<T>, std::greater<T>> queue;

	queue.push(std::make_pair(0, m_startFace));
	m_done.insert(std::make_pair(m_startFace, nullptr));

	while (!queue.empty())
	{
		auto pair = queue.top();
		auto* face = pair.second;
		queue.pop();

		for (auto& edge : face->GetEdges())
		{
			if (edge.twin)
			{
				auto* next = edge.twin->face;
				if (next != face && m_done.count(next) == 0)
				{
					if (next == m_endFace)
					{
						FacePath path = { next, face };
						while (face = m_done[face])
							path.push_back(face);

						return path;
					}
						
					queue.push(std::make_pair(GetPriority(*next), next));
					m_done.insert(std::make_pair(next, face));
				}
			}
		}
	}
	return FacePath();
}
