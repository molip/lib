#include "Line2.h"

#include <utility>

using namespace Jig;

Line2::Line2() : m_c(0), m_m(0), m_vert(false), m_valid(false) 
{
}

Line2::Line2(const Vec2& p0, const Vec2& p1) : m_p0(p0), m_p1(p1), m_c(0), m_m(0), m_vert(false), m_valid(false) 
{
}

Line2::Line2(const Vec2& p0, double m) : m_p0(p0), m_m(m), m_vert(false), m_valid(true)
{
	m_c = p0.y - m * p0.x;
	m_p1.x = 0; 
	m_p1.y = m_c;
}

Line2 Line2::MakeVertical(double x)
{
	return Line2(Vec2(x, 0), Vec2(x, 1));
}

Line2 Line2::MakeHorizontal(double y)
{
	return Line2(Vec2(0, y), Vec2(1, y));
}

const Line2& Line2::operator=(const Line2& rhs)
{
	std::swap(*this, Line2(rhs));
	return *this;
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
		
double Line2::Length() const // finite
{
	Vec2 v = m_p1 - m_p0;
	return sqrt(v.x*v.x + v.y*v.y);
}	

bool Line2::Intersect(const Line2& that, Vec2& point) const // infinite
{
	Validate();
	that.Validate();

	if (IsVertical())
	{
		if (that.IsVertical())
			return false;
		point.x = m_p0.x;
		point.y = that.m_c + that.m_m*point.x;
		return true;
	}
	else if (that.IsVertical())
		return that.Intersect(*this, point);		
		
	double diffM = m_m - that.m_m;
	if (fabs(diffM) < Epsilon)
		return false;
	point.x = (that.m_c-m_c) / (diffM);
	point.y = (m_m*point.x) + m_c;
	return true;
}
	
Line2 Line2::GetPerpBisector() const // finite
{
	Validate();

	Vec2 centre = m_p0 + (m_p1 - m_p0) / 2.0f;
	if (IsVertical())
		return Line2(centre, 0);
	else if (IsHorizontal())
		return MakeVertical(centre.x);
	else
		return Line2(centre, -1/m_m);
}

double Line2::DistanceTo(const Vec2& point, bool finite) const
{
	Validate();


	if (IsVertical())
		return point.x - m_p0.x;
	if (IsHorizontal())
		return point.y - m_p0.y;
	Line2 perp(point, -1/m_m);
	Vec2 ip;
	bool ok = Intersect(perp, ip);
	assert(ok);

	if (finite)
	{
		double length = Length();
		double dp0 = Jig::Vec2(ip-m_p0).GetLength();
		double dp1 = Jig::Vec2(ip - m_p1).GetLength();
		if (dp0 > length || dp1 > length)
			return (dp0<dp1) ? dp0 : dp1;
	}		
	perp.m_p1 = ip;
	return perp.Length();
}

bool Line2::IsHorizontal() const
{
	return !IsVertical() && fabs(m_m) < Epsilon;
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
