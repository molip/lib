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

		typedef std::vector<Vec2> Path;

		Path Find();
	private:
		struct Limit 
		{
			Limit() {}
			Limit(const Vec2& anchor, const Vec2& point);
			Vec2 point, vec; 
		};

		typedef std::pair<Limit, Limit> Limits;
		Limits GetLimits(const Vec2& anchor, const EdgeMesh::Edge& edge) const;
		const Vec2* GetNewAnchor(const Limits& limits, const Vec2& newVec0, const Vec2& newVec1) const;

		double GetPriority(const EdgeMesh::Face& face);
		double GetPathToStart(const Vec2& point, const EdgeMesh::Face& face, Path* path = nullptr);
			
		const Vec2 m_startPoint, m_endPoint;
		const EdgeMesh::Face* m_startFace;
		const EdgeMesh::Face* m_endFace;
		std::map<const EdgeMesh::Face*, const EdgeMesh::Edge*> m_done; // Face -> entering edge on face. 
	};
}
