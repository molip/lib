#pragma once

#include "EdgeMesh.h"

#include <map>
#include <vector>

namespace Jig
{
	
	class PathFinder
	{
	public:
		PathFinder(const EdgeMesh& mesh, const Vec2& startPoint, const Vec2& endPoint);
		~PathFinder();

		typedef std::vector<const EdgeMesh::Face*> FacePath;

		FacePath Find();
	private:
		double GetPriority(const EdgeMesh::Face& face);
		double GetG(const Vec2& point);
			
		const Vec2 m_startPoint, m_endPoint;
		const EdgeMesh::Face* m_startFace;
		const EdgeMesh::Face* m_endFace;
		std::map<const EdgeMesh::Face*, const EdgeMesh::Face*> m_done; // Face -> previous.
	};
}
