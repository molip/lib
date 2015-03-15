#pragma once

#include "EdgeMesh.h"
#include "Rect.h"
#include "Vector.h"

#include <vector>

namespace Jig
{
	class Polygon : public std::vector<Vec2>
	{
		friend class ShapeSplitter;

	public:
		Polygon();
		Polygon(Polygon&& rhs);
		~Polygon();

		Rect GetBBox() const;
		int AddPoint(const Vec2& point, double tolerance);

		const Vec2& GetVertex(int vert) const;
		bool IsCW() const;

		void Update(bool optimise);

		bool IsSelfIntersecting() const { return m_isSelfIntersecting; }
		const EdgeMesh& GetEdgeMesh() const { return m_mesh; }

	private:
		void MakeCW();
		double GetAngle(int vert) const;
		Vec2 GetVecTo(int vert) const;
		Vec2 GetVec(int from, int to) const;
		int ClampVertIndex(int vert) const;

		bool m_isSelfIntersecting;

		EdgeMesh m_mesh;

	};
}