#include "PathFinder.h"
#include "Debug.h"
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

PathFinder::Limits PathFinder::GetLimits(const Vec2& anchor, const EdgeMesh::Edge& edge) const
{
	return std::make_pair(Limit(anchor, edge.GetLine().GetP0()), Limit(anchor, edge.GetLine().GetP1()));
}

const Vec2* PathFinder::GetNewAnchor(const Limits& limits, const Vec2& newVec0, const Vec2& newVec1) const
{
	Vec2* newAnchor = nullptr;
	assert(!newVec0.IsZero() && !newVec1.IsZero());
	if (limits.first.vec.GetAngle(newVec1) < 0)
	{
		return &limits.first.point;
		Debug::Trace << "point0 is corner" << std::endl;
	}

	else if (limits.second.vec.GetAngle(newVec0) > 0)
	{
		return &limits.second.point;
		Debug::Trace << "point1 is corner" << std::endl;
	}
	return nullptr;
}

double PathFinder::GetPathToStart(const Vec2& point, const EdgeMesh::Face& face, Path* path)
{
	Debug::Trace << "PathFinder::GetPathToStart" << std::endl;

	if (path)
		path->push_back(point);

	auto* edge = m_done[&face];

	Limits limits;
	Vec2 anchor = point;
	double pathLength = 0;
	do
	{
		Limits newLimits = GetLimits(anchor, *edge);

		Debug::Trace << "anchor: " << anchor.x << "," << anchor.y << std::endl;
		Debug::Trace << "old limits: " << limits.first.point.x << "," << limits.first.point.y << " " <<
			limits.second.point.x << "," << limits.second.point.y << std::endl;
		Debug::Trace << "new limits: " << newLimits.first.point.x << "," << newLimits.first.point.y << " " <<
			newLimits.second.point.x << "," << newLimits.second.point.y << std::endl;

		if (limits.first.vec.IsZero() || limits.second.vec.IsZero()) // Anchor is on vertex shared with next face.
		{
			limits = newLimits;
			Debug::Trace << "Anchor on limit - skipping"<< std::endl;
		}
		else
		{
			if (const Vec2* newAnchor = GetNewAnchor(limits, newLimits.first.vec, newLimits.second.vec))
			{
				pathLength += Vec2(anchor - *newAnchor).GetLength();
				anchor = *newAnchor;
				limits = GetLimits(anchor, *edge);
				if (path)
					path->push_back(anchor);
			}
			else
			{
				if (limits.first.vec.GetAngle(newLimits.first.vec) > 0) // limits.first narrowed.
				{
					limits.first = newLimits.first;
					Debug::Trace << "limits.first narrowed" << std::endl;
				}
				if (limits.second.vec.GetAngle(newLimits.second.vec) < 0) // limits.second narrowed.
				{
					limits.second = newLimits.second;
					Debug::Trace << "limits.second narrowed" << std::endl;
				}
			}
		}

	} while (edge = m_done[edge->twin->face]);

	if (!limits.first.vec.IsZero() && !limits.second.vec.IsZero()) // Anchor isn't on vertex of start face.
	{
		Vec2 vecToStart(m_startPoint - anchor);
		if (vecToStart.Normalise())
		{
			if (const Vec2* newAnchor = GetNewAnchor(limits, vecToStart, vecToStart))
			{
				pathLength += Vec2(anchor - *newAnchor).GetLength();
				anchor = *newAnchor;
				if (path)
					path->push_back(anchor);
			}
		}
	}
	pathLength += Vec2(anchor - m_startPoint).GetLength();
	if (path)
		path->push_back(m_startPoint);

	return pathLength;
}

double PathFinder::GetPriority(const EdgeMesh::Face& face)
{
	double h = 0;
	Vec2 point = Geometry::GetClosestPoint(face.GetLineLoop(), m_endPoint, &h);
	return h + GetPathToStart(point, face);
}

PathFinder::Path PathFinder::Find()
{
	if (!m_startFace || !m_endFace)
		return Path();

	if (m_startFace == m_endFace)
		return Path{ m_endPoint, m_startPoint };
	
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
					m_done.insert(std::make_pair(next, edge.twin));
					
					if (next == m_endFace)
					{
						Path path;
						GetPathToStart(m_endPoint, *m_endFace, &path);
						return path;
					}
						
					queue.push(std::make_pair(GetPriority(*next), next));
				}
			}
		}
	}
	return Path();
}

//-----------------------------------------------------------------------------

PathFinder::Limit::Limit(const Vec2& _anchor, const Vec2& _point) : point(_point), vec(_point - _anchor) 
{
	vec.Normalise();
}
