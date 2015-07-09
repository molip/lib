#include "PolyLine.h"

#include "Geometry.h"
#include "Line2.h"
#include "Vector.h"

#include "libKernel/Serial.h"

#include <algorithm>
#include <map>
#include <numeric>

using namespace Jig;

PolyLine::PolyLine() : m_isSelfIntersecting(false), m_isClosed(false)
{
}

PolyLine::~PolyLine()
{
}

Rect PolyLine::GetBBox() const
{
	return Geometry::GetBBox(*this);
}

bool PolyLine::IsCW() const
{
	KERNEL_ASSERT(m_isClosed);

	double total = 0.f;
	for (int i = 0; i < (int)size(); ++i)
		total += GetAngle(i);

	return total > 0;
}

void PolyLine::MakeCW()
{
	if (!IsCW())
		std::reverse(begin(), end());
}

int PolyLine::AddPoint(const Vec2& point, double tolerance)
{
	int minEdge = -1;
	double minDist = 1 << 16;

	for (int i = 0; i < GetSegmentCount(); ++i)
	{
		auto edge = Line2::MakeFinite(GetVertex(i), GetVertex(i + 1));

		bool intersects = false;
		double dist = 0;
		if (edge.PerpIntersect(Vec2(point.x, point.y), &dist))
			if (tolerance > dist && minDist > dist)
				minDist = dist, minEdge = i;
	}

	if (minEdge >= 0)
		insert(begin() + minEdge + 1, point);
	
	return minEdge >= 0 ? minEdge + 1 : -1;
}

int PolyLine::ClampVertIndex(int vert) const
{
	if (!m_isClosed)
	{
		KERNEL_ASSERT(vert >= 0 && vert < (int)size());
		return vert;
	}

	return
		vert < 0 ? vert + (int)size() :
		vert >= (int)size() ? vert - (int)size() :
		vert;
}

const Vec2& PolyLine::GetVertex(int vert) const
{
	return at(ClampVertIndex(vert));
}

Vec2 PolyLine::GetVecTo(int vert) const
{
	return GetVec(vert - 1, vert);
}

Vec2 PolyLine::GetVec(int from, int to) const
{
	Vec2 p = at(ClampVertIndex(from));
	Vec2 q = at(ClampVertIndex(to));

	return q - p;
}

void PolyLine::RemoveHoles()
{
	KERNEL_ASSERT(m_isClosed);
	
	std::vector<int> indices(size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(), [&](int lhs, int rhs) { return at(lhs) < at(rhs); });

	for (;;)
	{
		// TODO: Do we need to start from begin() every time? 
		auto it = std::adjacent_find(indices.begin(), indices.end(), [&](int lhs, int rhs) { return at(lhs) == at(rhs); });
		if (it == indices.end())
			break;

		int first = *it, second = *(it + 1);
		if (first > second)
			std::swap(first, second);

		auto EraseIndices = [&](bool outer) { indices.erase(std::remove_if(indices.begin(), indices.end(), [&](int i) { return (i >= first && i < second) ^ outer; }), indices.end()); };

		// TODO: Check for cw/ccw instead. 
		if (second - first < size() / 2) // Erase [first, second).
		{
			erase(begin() + first, begin() + second);

			EraseIndices(false);

			for (int& i : indices)
				if (i >= second)
					i -= second - first;
		}
		else // Keep [first, second).
		{
			erase(begin() + second, end());
			erase(begin(), begin() + first);

			EraseIndices(true);

			for (int& i : indices)
				i -= first;
		}

		KERNEL_VERIFY(indices.size() == size());
	}
}

double PolyLine::GetAngle(int vert) const
{
	Vec2 v0 = GetVecTo(vert).Normalised();
	Vec2 v1 = GetVecTo(vert + 1).Normalised();

	return v0.GetAngle(v1);
}

void PolyLine::Update()
{
	if (m_isClosed)
		MakeCW();
	
	m_isSelfIntersecting = false;

	for (int i = 0; i < GetSegmentCount(); ++i)
	{
		int last = (int)size();
		if (m_isClosed && i == 0)
			--last; // Don't intersect last edge with first.

		auto edge0 = Line2::MakeFinite(GetVertex(i), GetVertex(i + 1));
		for (int j = i + 2; j < last; ++j) 
		{
			auto edge1 = Line2::MakeFinite(GetVertex(j), GetVertex(j + 1));
			if (m_isSelfIntersecting = edge0.Intersect(edge1))
				break;
		}
		if (m_isSelfIntersecting)
			break;
	}
}

bool PolyLine::Contains(const Vec2& point) const
{
	return Geometry::PointInPolygon(GetPointPairLoop(), point);
}

void PolyLine::Save(Kernel::Serial::SaveNode& node) const
{
	node.SaveCntr("points", *this, Kernel::Serial::TypeSaver());
	node.SaveType("is_self_intersecting", m_isSelfIntersecting);
	node.SaveType("is_closed", m_isClosed);
}

void PolyLine::Load(const Kernel::Serial::LoadNode& node)
{
	node.LoadCntr("points", *this, Kernel::Serial::TypeLoader());
	node.LoadType("is_self_intersecting", m_isSelfIntersecting);
	
	bool closed{};
	if (node.LoadType("is_closed", closed))
		m_isClosed = closed;
}
