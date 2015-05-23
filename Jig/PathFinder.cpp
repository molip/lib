#include "PathFinder.h"
#include "Debug.h"
#include "Geometry.h"

#include <queue>
#include <functional>

using namespace Jig;

bool PathFinder::Envelope::UpdateAnchor(const Vec2& vec, Vec2& anchor, double& dist, Path* path) const
{
	int iLastLimit = FindLastNarrowerLimit(vec);
	if (iLastLimit < 0)
		return false;

	Vec2 p = anchor;

	for (size_t i = 0; i < iLastLimit; ++i)
	{
		Vec2 q = m_limits[i].point;
		dist += Vec2(q - p).GetLength();
		if (path)
			path->push_back(q);
		p = q;
	}

	Vec2 q = m_limits[iLastLimit].point;
	dist += Vec2(q - p).GetLength();
	if (path)
		path->push_back(q);

	anchor = q;
	return true;
}

void PathFinder::Envelope::Add(const Limit& limit)
{
	// Roll back wider limits.
	int iLastLimit = FindLastNarrowerLimit(limit.vec);
	m_limits.erase(m_limits.begin() + (iLastLimit + 1), m_limits.end());
	m_limits.push_back(limit);
}

void PathFinder::Envelope::Reset(const Limit& limit)
{
	m_limits = { limit };
}

void PathFinder::Envelope::Clear()
{
	m_limits.clear();
}

int PathFinder::Envelope::FindLastNarrowerLimit(const Vec2& vec) const
{
	for (int i = (int)m_limits.size() - 1; i >= 0; --i)
		if (Compare(m_limits[i].vec, vec))
			return i;
	return -1;
}

bool PathFinder::Envelope::Compare(const Vec2& v0, const Vec2& v1) const
{
	double angle = v0.GetAngle(v1);
	return m_second ? angle > 0 : angle < 0;
}



PathFinder::PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint) : m_startPoint(startPoint), m_endPoint(endPoint)
{
	m_startFace = mesh.HitTest(startPoint);
	m_endFace = mesh.HitTest(endPoint);
}

PathFinder::~PathFinder()
{
}

double PathFinder::GetPathToStart(const Vec2& point, const EdgeMesh::Face& face, Path* path)
{
	Debug::Trace << "PathFinder::GetPathToStart" << std::endl;

	if (path)
		path->push_back(point);

	auto* edge = m_done[&face];
	Envelope env0(false), env1(true);

	Vec2 anchor = point;
	double pathLength = 0;
	while (edge)
	{
		const Limit limit0(anchor, edge->GetLine().GetP0());
		const Limit limit1(anchor, edge->GetLine().GetP1());

		Debug::Trace << "anchor: " << anchor.x << "," << anchor.y << std::endl;
		Debug::Trace << "limits: " << limit0.point.x << "," << limit0.point.y << " " <<
			limit1.point.x << "," << limit1.point.y << std::endl;

		if (limit0.vec.IsZero() || limit1.vec.IsZero()) // Anchor is on vertex shared with next face.
		{
			env0.Clear();
			env1.Clear();
			Debug::Trace << "Anchor on limit - skipping"<< std::endl;
		}
		else
		{
			if (env0.UpdateAnchor(limit1.vec, anchor, pathLength, path) || env1.UpdateAnchor(limit0.vec, anchor, pathLength, path))
			{
				// Anchor has changed.
				env0.Clear();
				env1.Clear();
				continue; // Re-evaluate current edge with new anchor. 
			}
			else
			{
				env0.Add(limit0);
				env1.Add(limit1);
			}
		}
		edge = m_done[edge->twin->face];
	}

	Vec2 vecToStart(m_startPoint - anchor);
	if (vecToStart.Normalise())
	{
		if (!env0.UpdateAnchor(vecToStart, anchor, pathLength, path))
			env1.UpdateAnchor(vecToStart, anchor, pathLength, path);
			
		pathLength += Vec2(anchor - m_startPoint).GetLength();
		if (path)
			path->push_back(m_startPoint);
	}

	return pathLength;
}

double PathFinder::GetPriority(const EdgeMesh::Face& face)
{
	double h = 0;
	Vec2 point = Geometry::GetClosestPoint(face.GetLineLoop(), m_endPoint, &h);
	return h + GetPathToStart(point, face);
}

PathFinder::Path PathFinder::Find(double* length)
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
						double pathLength = GetPathToStart(m_endPoint, *m_endFace, &path);
						if (length)
							*length = pathLength;
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
