#include "PathFinder.h"
#include "libKernel/Debug.h"
#include "Geometry.h"
#include "GetVisiblePoints.h"

using namespace Jig;
using namespace Kernel;

PathFinder::PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint) : m_mesh(mesh), m_startPoint(startPoint), m_endPoint(endPoint), m_isFinished(false), m_length(0)
{
	if (IsVisible(m_mesh, m_startPoint, m_endPoint))
	{
		m_length = Vec2(m_endPoint - m_startPoint).GetLength();
		m_path = { m_endPoint, m_startPoint };
		m_isFinished = true;
		return;
	}

	EdgeMesh::VertPtrVec startVisible = GetVisiblePoints(m_mesh, m_startPoint);
	EdgeMesh::VertPtrVec endVisible = GetVisiblePoints(m_mesh, m_endPoint);

	if (startVisible.empty() || endVisible.empty())
	{
		m_isFinished = true;
		return;
	}

	m_endVisibleSet.insert(endVisible.begin(), endVisible.end());

	for (auto* v : startVisible)
		AddVert(v, nullptr);
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
		m_queue.insert(std::make_pair(g + h, vert));
	}
}

void PathFinder::Go()
{
	while (!IsFinished())
		Step();
}

void PathFinder::Step()
{
	if (IsFinished())
		return;

	if (m_queue.empty())
	{
		m_path.clear();
		m_length = 0;
		m_isFinished = true;
		return;
	}

	auto pair = m_queue.begin();
	auto* vert = pair->second;
	m_queue.erase(pair);

	m_path.clear();
	m_length = GetPathToStart(vert, &m_path);

	if (m_endVisibleSet.count(vert))
	{
		m_path.insert(m_path.begin(), m_endPoint);
		m_length += Vec2(m_endPoint - *vert).GetLength();

		m_isFinished = true;
		return;
	}

	for (auto* next : vert->visible)
		AddVert(next, vert);
}

