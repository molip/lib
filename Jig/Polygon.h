#pragma once

#include "EdgeMesh.h"
#include "Rect.h"
#include "Vector.h"

#include "libKernel/Util.h"

#include <vector>

namespace Jig
{
	class Polygon : public std::vector<Vec2>
	{
	public:
		class Iter
		{
		public:
			Iter(const Polygon& poly, bool end) : m_poly(poly), m_index(end ? poly.size() : 0) {}
			bool operator !=(const Iter& rhs) const { return m_index != rhs.m_index; }
			void operator++ () { ++m_index; }

		protected:
			const Polygon& m_poly;
			size_t m_index;
		};

		class LineIter : public Iter
		{
		public:
			using Iter::Iter;
			Line2 operator* () const { return Line2::MakeFinite(m_poly[m_index], m_poly[(m_index + 1) % m_poly.size()]); }
		};

		class PointPairIter : public Iter
		{
		public:
			using Iter::Iter;
			std::pair<Vec2, Vec2> operator* () const { return std::make_pair(m_poly[m_index], m_poly[(m_index + 1) % m_poly.size()]); }
		};

		Polygon();
		~Polygon();

		Rect GetBBox() const;
		int AddPoint(const Vec2& point, double tolerance);
		Jig::Polygon GetInflated(double val);

		const Vec2& GetVertex(int vert) const;
		bool IsCW() const;
		void MakeCW();

		void Update();

		bool IsSelfIntersecting() const { return m_isSelfIntersecting; }
		bool Contains(const Vec2& point) const;

		Kernel::Iterable<LineIter> GetLineLoop() const { return Kernel::Iterable<LineIter>(LineIter(*this, false), LineIter(*this, true)); }
		Kernel::Iterable<PointPairIter> GetPointPairLoop() const { return Kernel::Iterable<PointPairIter>(PointPairIter(*this, false), PointPairIter(*this, true)); }

		void Save(Kernel::Serial::SaveNode& node) const;
		void Load(const Kernel::Serial::LoadNode& node);

	private:
		double GetAngle(int vert) const;
		Vec2 GetNormal(int vert) const;
		Vec2 GetVecTo(int vert) const;
		Vec2 GetVec(int from, int to) const;
		int ClampVertIndex(int vert) const;

		bool m_isSelfIntersecting;
	};
}