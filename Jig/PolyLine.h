#pragma once

#include "EdgeMesh.h"
#include "Rect.h"
#include "Vector.h"

#include "libKernel/Util.h"

#include <vector>

namespace Jig
{
	class PolyLine : public std::vector<Vec2>
	{
	public:
		class Iter
		{
		public:
			Iter(const PolyLine& poly, bool end) : m_poly(poly), m_index(end ? poly.GetSegmentCount() : 0) {}
			bool operator !=(const Iter& rhs) const { return m_index != rhs.m_index; }
			void operator++ () { ++m_index; }

		protected:
			const PolyLine& m_poly;
			size_t m_index;
		};

		class LineIter : public Iter
		{
		public:
			using Iter::Iter;
			Line2 operator* () const { return Line2::MakeFinite(m_poly.GetVertex((int)m_index), m_poly.GetVertex((int)m_index + 1)); }
		};

		class PointPairIter : public Iter
		{
		public:
			using Iter::Iter;
			std::pair<Vec2, Vec2> operator* () const { return std::make_pair(m_poly.GetVertex((int)m_index), m_poly.GetVertex((int)m_index + 1)); }
		};

		PolyLine();
		~PolyLine();

		Rect GetBBox() const;
		int AddPoint(const Vec2& point, double tolerance);

		const Vec2& GetVertex(int vert) const;
		bool IsCW() const;
		void MakeCW();

		void Update();

		void SetClosed(bool val) { m_isClosed = val; }
		bool IsClosed() const { return m_isClosed; }

		int GetSegmentCount() const { return (int)size() - m_isClosed ? 0 : 1; }
		bool IsSelfIntersecting() const { return m_isSelfIntersecting; }
		bool Contains(const Vec2& point) const;

		Kernel::Iterable<LineIter> GetLineLoop() const { return Kernel::Iterable<LineIter>(LineIter(*this, false), LineIter(*this, true)); }
		Kernel::Iterable<PointPairIter> GetPointPairLoop() const { return Kernel::Iterable<PointPairIter>(PointPairIter(*this, false), PointPairIter(*this, true)); }

		void Save(Kernel::Serial::SaveNode& node) const;
		void Load(const Kernel::Serial::LoadNode& node);

	private:
		virtual int ClampVertIndex(int vert) const;

		double GetAngle(int vert) const;
		Vec2 GetVecTo(int vert) const;
		Vec2 GetVec(int from, int to) const;

		bool m_isSelfIntersecting;
		bool m_isClosed;
	};
}