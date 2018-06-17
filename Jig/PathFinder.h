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

		using VertPtr = const EdgeMesh::Vert*;

		struct QueueItem
		{
			bool operator <(const QueueItem& rhs) const
			{
				return std::make_tuple(gLength + hLength, vert, prev) > std::make_tuple(rhs.gLength + rhs.hLength, rhs.vert, prev); // Reverse sort. 
			}

			double gLength; // Along path to start.
			double hLength; // Straight line to end.
			VertPtr vert, prev; // prev is only used to avoid sorting collisions. 
		};

		struct DoneItem
		{
			double length; // Along path to start.
			VertPtr prev;
		};

		typedef std::vector<Vec2> Path;
		typedef std::map<VertPtr, DoneItem> DoneMap;
		
		class Queue : public std::priority_queue<QueueItem>
		{
		public:
			const container_type& GetContainer() const { return c; }
		};

		bool IsFinished() const { return m_isFinished; }

		Path GetPath() const;
		double GetLength() const { return m_length; }

		const Queue::container_type& GetQueue() const { return m_queue.GetContainer(); }
		const DoneMap & GetDone() const { return m_done; }

		void Go();
		void Step();

	private:
		void AppendPathToStart(VertPtr vert, PathFinder::Path& path) const;
		void AddVert(VertPtr vert, VertPtr prev, double prevLength);

		const EdgeMesh& m_mesh;
		const Vec2 m_startPoint, m_endPoint;
		bool m_isFinished;
		Path m_path;
		double m_length;
		VertPtr m_currentVert;
		
		Queue m_queue;
		DoneMap m_done;
		std::set<VertPtr> m_endVisibleSet;
	};
}
