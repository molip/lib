#include "PathFinder.h"
#include "EdgeMeshVisibility.h"
#include "Geometry.h"
#include "GetVisiblePoints.h"

#include "libKernel/Debug.h"

using namespace Jig;
using namespace Kernel;

PathFinder::PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint) : m_mesh(mesh), m_startPoint(startPoint), m_endPoint(endPoint), m_isFinished(false), m_length(0), m_currentVert(nullptr)
{
	if (IsVisible(m_mesh, m_startPoint, m_endPoint))
	{
		m_length = Vec2(m_endPoint - m_startPoint).GetLength();
		m_path = { m_endPoint, m_startPoint };
		m_isFinished = true;
		return;
	}

	std::vector<const EdgeMesh::Vert*> startVisible = GetVisiblePoints(m_mesh, m_startPoint);
	std::vector<const EdgeMesh::Vert*> endVisible = GetVisiblePoints(m_mesh, m_endPoint);

	if (startVisible.empty() || endVisible.empty())
	{
		m_isFinished = true;
		return;
	}

	m_endVisibleSet.insert(endVisible.begin(), endVisible.end());

	for (auto* v : startVisible)
		AddVert(v, nullptr, 0);
}

PathFinder::~PathFinder()
{
}

void PathFinder::AppendPathToStart(VertPtr vert, PathFinder::Path& path) const
{
	path.push_back(*vert);

	while (VertPtr prev = m_done.find(vert)->second.prev)
	{
		path.push_back(*prev);
		vert = prev;
	}
	
	path.push_back(m_startPoint);
}

void PathFinder::AddVert(VertPtr vert, VertPtr prev, double prevLength)
{
	const double length = prevLength + Vec2(*vert - (prev ? *prev : m_startPoint)).GetLength();

	// Already added this vert? 
	auto pair = m_done.insert(std::make_pair(vert, DoneItem{}));
	DoneItem& item = pair.first->second;
	if (pair.second || length < item.length) // New or shorter. 
	{
		// Initialise or update. 
		item.length = length;
		item.prev = prev;

		// Add to queue.
		double g = length;
		double h = Vec2(m_endPoint - *vert).GetLength();
		m_queue.push(QueueItem{ g, h, vert, prev }); // vert might already be in the queue, with a lower priority. 
	}
}

PathFinder::Path PathFinder::GetPath() const 
{
	if (IsFinished())
		return m_path;

	PathFinder::Path path;
	if (m_currentVert)
		AppendPathToStart(m_currentVert, path);
	
	return path; // Best path so far. 
}

void PathFinder::Go()
{
	while (!IsFinished())
		Step();
}

void PathFinder::Step()
{
	KERNEL_ASSERT(!IsFinished());

	if (m_queue.empty())
	{
		m_path.clear();
		m_length = 0;
		m_isFinished = true;
		return;
	}

	const QueueItem item = m_queue.top();
	m_queue.pop();

	m_currentVert = item.vert;
	m_length = item.gLength;

	if (m_endVisibleSet.count(item.vert))
	{
		KERNEL_ASSERT(m_path.empty());
		m_path.push_back(m_endPoint);
		AppendPathToStart(item.vert, m_path);

		m_length += Vec2(m_endPoint - *item.vert).GetLength();

		m_isFinished = true;
		return;
	}

	for (auto* next : EdgeMeshVisibility::GetData(item.vert)->visible)
		AddVert(next, item.vert, item.gLength);
}

