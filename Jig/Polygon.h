#pragma once

#include "EdgeMesh.h"
#include "Rect.h"
#include "Util.h"
#include "Vector.h"

#include <vector>

namespace Jig
{
	class Polygon : public std::vector<Vec2>
	{
		friend class ShapeSplitter;

	public:
		class LineIter
		{
		public:
			LineIter(const Polygon& poly, bool end) : m_poly(poly), m_index(end ? poly.size() : 0) {}
			bool operator !=(const LineIter& rhs) const { return m_index != rhs.m_index; }
			void operator++ () { ++m_index; }
			Line2 operator* () const { return Line2::MakeFinite(m_poly[m_index], m_poly[(m_index + 1) % m_poly.size()]); }

		private:
			const Polygon& m_poly;
			size_t m_index;
		};

		Polygon();
		Polygon(Polygon&& rhs);
		~Polygon();

		Rect GetBBox() const;
		int AddPoint(const Vec2& point, double tolerance);

		const Vec2& GetVertex(int vert) const;
		bool IsCW() const;
		void MakeCW();

		void Update();

		bool IsSelfIntersecting() const { return m_isSelfIntersecting; }
		bool Contains(const Vec2& point) const;

		Util::Iterable<LineIter> GetLineLoop() const { return Util::Iterable<LineIter>(LineIter(*this, false), LineIter(*this, true)); }

	private:
		double GetAngle(int vert) const;
		Vec2 GetVecTo(int vert) const;
		Vec2 GetVec(int from, int to) const;
		int ClampVertIndex(int vert) const;

		bool m_isSelfIntersecting;
	};
}