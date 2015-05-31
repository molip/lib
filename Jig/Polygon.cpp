#include "Polygon.h"

#include "ShapeSplitter.h"

#include "Geometry.h"
#include "Line2.h"
#include "Util.h"
#include "Vector.h"

#include <algorithm>
#include <map>

using namespace Jig;

Polygon::Polygon() 
{
}

Polygon::~Polygon()
{
}

Rect Polygon::GetBBox() const
{
	return Geometry::GetBBox(*this);
}

bool Polygon::IsCW() const
{
	double total = 0.f;
	for (int i = 0; i < (int)size(); ++i)
		total += GetAngle(i);

	return total > 0;
}

void Polygon::MakeCW()
{
	if (!IsCW())
		std::reverse(begin(), end());
}

int Polygon::AddPoint(const Vec2& point, double tolerance)
{
	int minEdge = -1;
	double minDist = 1 << 16;

	for (int i = 0; i < (int)size(); ++i)
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

int Polygon::ClampVertIndex(int vert) const
{
	return	
		vert < 0 ? vert + (int)size() : 
		vert >= (int)size() ? vert - (int)size() :
		vert;
}

const Vec2& Polygon::GetVertex(int vert) const
{
	return at(ClampVertIndex(vert));
}

Vec2 Polygon::GetVecTo(int vert) const
{
	return GetVec(vert - 1, vert);
}

Vec2 Polygon::GetVec(int from, int to) const
{
	Vec2 p = at(ClampVertIndex(from));
	Vec2 q = at(ClampVertIndex(to));

	return q - p;
}

double Polygon::GetAngle(int vert) const
{
	Vec2 v0 = GetVecTo(vert).Normalised();
	Vec2 v1 = GetVecTo(vert + 1).Normalised();

	return v0.GetAngle(v1);
}

void Polygon::Update()
{
	MakeCW();

	m_isSelfIntersecting = false;

	for (int i = 0; i < (int)size(); ++i)
	{
		auto edge0 = Line2::MakeFinite(GetVertex(i), GetVertex(i + 1));
		for (int j = i + 2; j < (int)size() - (i == 0); ++j) // Don't intersect last edge with first.
		{
			auto edge1 = Line2::MakeFinite(GetVertex(j), GetVertex(j + 1));
			if (m_isSelfIntersecting = edge0.Intersect(edge1))
				break;
		}
		if (m_isSelfIntersecting)
			break;
	}
}

bool Polygon::Contains(const Vec2& point) const
{
	return Geometry::PointInPolygon(GetLineLoop(), point);
}

