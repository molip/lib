#pragma once

#include "EdgeMesh.h"

#include <functional>
#include <map>
#include <queue>
#include <vector>

namespace Jig
{
	
	class PathFinder
	{
	public:
		PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint);
		~PathFinder();

		typedef std::vector<Vec2> Path;

		Path Find(double* length = nullptr);

	private:
		double GetPathToStart(EdgeMesh::VertPtr vert, Path* path = nullptr);
		void AddVert(EdgeMesh::VertPtr vert, EdgeMesh::VertPtr prev);

		const EdgeMesh& m_mesh;
		const Vec2 m_startPoint, m_endPoint;

		typedef std::pair<double, EdgeMesh::VertPtr> T;
		std::priority_queue<T, std::vector<T>, std::greater<T>> m_queue;
		std::map<EdgeMesh::VertPtr, EdgeMesh::VertPtr> m_done;
	};
}
