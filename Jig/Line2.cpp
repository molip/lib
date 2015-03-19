#include "Line2.h"

#include "Rect.h"

#include <utility>

using namespace Jig;

Line2::Line2() : m_c(0), m_m(0), m_vert(false), m_valid(false) 
{
}

Line2::Line2(const Vec2& p0, const Vec2& p1, bool finite) : m_p0(p0), m_p1(p1), m_finite(finite), m_c(0), m_m(0), m_vert(false), m_valid(false)
{
}

Line2::Line2(const Vec2& p0, double m) : m_p0(p0), m_m(m), m_vert(false), m_valid(true), m_finite(false)
{
	m_c = p0.y - m * p0.x;
	m_p1.x = 0; 
	m_p1.y = m_c;
}

Line2 Line2::MakeFinite(const Vec2& p0, const Vec2& p1)
{
	return Line2(p0, p1, true);
}

Line2 Line2::MakeInfinite(const Vec2& p0, const Vec2& p1)
{
	return Line2(p0, p1, true);
}

Line2 Line2::MakeVertical(double x)
{
	return Line2(Vec2(x, 0), Vec2(x, 1), false);
}

Line2 Line2::MakeHorizontal(double y)
{
	return Line2(Vec2(0, y), Vec2(1, y), false);
}

void Line2::SwapPoints()
{
	std::swap(m_p0, m_p1);
	m_valid = false;
}

void Line2::SetP0(const Vec2& p0)
{
	m_p0 = p0; 
	m_valid = false;
}
	
void Line2::SetP1(const Vec2& p1)
{
	m_p1 = p1; 
	m_valid = false;
}
		
const Vec2& Line2::GetP0() const
{
	if (!m_finite)
		throw;
	return m_p0;
}

const Vec2& Line2::GetP1() const
{ 
	if (!m_finite)
		throw;
	return m_p1;
}

double Line2::Length() const
{
	if (!m_finite)
		throw;
	Vec2 v = m_p1 - m_p0;
	return sqrt(v.x*v.x + v.y*v.y);
}	

bool Line2::Intersect(const Line2& that, Vec2* pPoint) const
{
	Validate();
	that.Validate();

	if (that.IsVertical())
		return IsVertical() ? false : that.Intersect(*this, pPoint);

	assert(!that.IsVertical());

	Vec2 point;
	if (IsVertical())
	{
		point.x = m_p0.x;
		point.y = that.m_c + that.m_m * point.x;
	}
	else
	{
		double diffM = m_m - that.m_m;
		if (fabs(diffM) < Epsilon) // Parallel.
			return false;
		
		point.x = (that.m_c - m_c) / (diffM);
		point.y = (m_m*point.x) + m_c;
	}

	bool hit = true;

	if (m_finite)
		hit &= IsPointWithinFiniteRange(point);

	if (that.m_finite)
		hit &= that.IsPointWithinFiniteRange(point);

	if (pPoint)
		*pPoint = point;

	return hit;
}

Line2 Line2::GetPerpBisector() const 
{
	if (!m_finite)
		throw;

	Validate();

	Vec2 centre = m_p0 + (m_p1 - m_p0) / 2.0;
	if (IsVertical())
		return Line2(centre, 0);
	else if (IsHorizontal())
		return MakeVertical(centre.x);
	else
		return Line2(centre, -1/m_m);
}

Line2 Line2::GetPerpThroughtPoint(const Vec2& point) const
{
	Validate();

	if (IsVertical())
		return MakeHorizontal(point.y);
	if (IsHorizontal())
		return MakeVertical(point.x);

	return Line2(point, -1 / m_m);
}

double Line2::PerpDistanceTo(const Vec2& point, bool* intersects) const
{
	Validate();

	Line2 perp = GetPerpThroughtPoint(point);

	Vec2 ip;
	bool ok = Intersect(perp, &ip);

	if (intersects)
		*intersects = ok;

	return MakeFinite(point, ip).Length();
}

bool Line2::IsHorizontal() const
{
	Validate();
	return !IsVertical() && fabs(m_m) < Epsilon;
}

// Assumes point is on extrapolated line. 
bool Line2::IsPointWithinFiniteRange(const Vec2& point) const
{
	Validate();

	if (!m_finite)
		throw;

	Rect r(m_p0, m_p1);
	r.Normalise();

	if (r.Width() > r.Height())
		return r.m_p0.x <= point.x && point.x <= r.m_p1.x;

	return r.m_p0.y <= point.y && point.y <= r.m_p1.y;
}

void Line2::Validate() const
{
	if (m_valid)
		return;

	double diffX = m_p1.x - m_p0.x;
	m_vert = fabs(diffX) < Epsilon;

	m_m = m_vert ? 0 : (m_p1.y - m_p0.y) / diffX;
	m_c = m_vert ? 0 : m_p0.y - (m_m*m_p0.x);
	m_valid = true;
}
