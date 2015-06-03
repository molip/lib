#include "PathFinder.h"
#include "Debug.h"
#include "Geometry.h"
#include "GetVisiblePoints.h"

using namespace Jig;

PathFinder::PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint) : m_mesh(mesh), m_startPoint(startPoint), m_endPoint(endPoint)
{
}

PathFinder::~PathFinder()
{
}

double PathFinder::GetPathToStart(EdgeMesh::VertPtr vert, Path* path)
{
	if (path)
		path->push_back(*vert);

	double pathLength = 0;

	while (EdgeMesh::VertPtr prev = m_done[vert])
	{
		if (path)
			path->push_back(*prev);

		pathLength += Vec2(*prev - *vert).GetLength();
		vert = prev;
	}

	if (path)
		path->push_back(m_startPoint);

	pathLength += Vec2(m_startPoint - *vert).GetLength();
	return pathLength;
}

void PathFinder::AddVert(EdgeMesh::VertPtr vert, EdgeMesh::VertPtr prev)
{
	if (m_done.insert(std::make_pair(vert, prev)).second)
	{
		double g = GetPathToStart(vert);
		double h = Vec2(m_endPoint - *vert).GetLength();
		m_queue.push(std::make_pair(g + h, vert));
	}
}

PathFinder::Path PathFinder::Find(double* length)
{
	if (IsVisible(m_mesh, m_startPoint, m_endPoint))
		return Path{ m_endPoint, m_startPoint };

	EdgeMesh::VertPtrVec startVisible = GetVisiblePoints(m_mesh, m_startPoint);
	EdgeMesh::VertPtrVec endVisible = GetVisiblePoints(m_mesh, m_endPoint);

	if (startVisible.empty() || endVisible.empty())
		return Path();

	EdgeMesh::Vert endVert(m_endPoint);

	for (auto* v : endVisible)
		const_cast<EdgeMesh::Vert*>(v)->visible.push_back(&endVert);

	std::set<EdgeMesh::VertPtr> endVisibleSet;
	endVisibleSet.insert(endVisible.begin(), endVisible.end());

	for (auto* v : startVisible)
		AddVert(v, nullptr);

	while (!m_queue.empty())
	{
		auto pair = m_queue.top();
		auto* vert = pair.second;
		m_queue.pop();

		if (endVisibleSet.count(vert))
		{
			Path path;
			path.push_back(m_endPoint);

			double pathLength = GetPathToStart(vert, &path);

			if (length)
				*length = Vec2(m_endPoint - *vert).GetLength() + pathLength;

			for (auto* v : endVisible)
				const_cast<EdgeMesh::Vert*>(v)->visible.pop_back();

			return path;
		}

		for (auto* next : vert->visible)
			AddVert(next, vert);
	}

	for (auto* v : endVisible)
		const_cast<EdgeMesh::Vert*>(v)->visible.pop_back();

	return Path();
}
