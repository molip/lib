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

		Path Find(double* length = nullptr);

	private:
		struct Limit 
		{
			Limit() {}
			Limit(const Vec2& anchor, const Vec2& point);
			Vec2 point, vec; 
		};

		class Envelope
		{
		public:
			Envelope(bool second) : m_second(second) {}

			bool UpdateAnchor(const Vec2& vec, Vec2& anchor, double& dist, Path* path) const;
			void Add(const Limit& limit);
			void Reset(const Limit& limit);
			void Clear();

		private:
			int FindLastNarrowerLimit(const Vec2& vec) const; // Returns index of last limit that vec is wider than.
			bool Compare(const Vec2& v0, const Vec2& v1) const; // Returns true if v1 is wider than v0.

			bool m_second;
			std::vector<Limit> m_limits;
		};

		double GetPriority(const EdgeMesh::Face& face);
		double GetPathToStart(const Vec2& point, const EdgeMesh::Face& face, Path* path = nullptr);
			
		const Vec2 m_startPoint, m_endPoint;
		const EdgeMesh::Face* m_startFace;
		const EdgeMesh::Face* m_endFace;
		std::map<const EdgeMesh::Face*, const EdgeMesh::Edge*> m_done; // Face -> entering edge on face. 
	};
}
