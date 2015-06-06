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
		typedef std::map<EdgeMesh::VertPtr, EdgeMesh::VertPtr> DoneMap;
		typedef std::map<double, EdgeMesh::VertPtr> Queue;

		bool IsFinished() const { return m_isFinished; }

		Path GetPath() const { return m_path; }
		double GetLength() const { return m_length; }

		const Queue& GetQueue() const { return m_queue; }
		const DoneMap & GetDone() const { return m_done; }

		void Go();
		void Step();

	private:
		double GetPathToStart(EdgeMesh::VertPtr vert, Path* path = nullptr);
		void AddVert(EdgeMesh::VertPtr vert, EdgeMesh::VertPtr prev);

		const EdgeMesh& m_mesh;
		const Vec2 m_startPoint, m_endPoint;
		bool m_isFinished;
		Path m_path;
		double m_length;
		
		//typedef std::pair<double, EdgeMesh::VertPtr> T;
		//std::priority_queue<T, std::vector<T>, std::greater<T>> m_queue;
		Queue m_queue;
		DoneMap m_done;
		std::set<EdgeMesh::VertPtr> m_endVisibleSet;
	};
}
