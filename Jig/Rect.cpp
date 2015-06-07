#include "Rect.h"

using namespace Jig;

Rect::Rect()
{
}

Rect::Rect(const Vec2& p) : m_p0(p), m_p1(p)
{
}

Rect::Rect(const Vec2& p0, const Vec2& p1) : m_p0(p0), m_p1(p1)
{
}

Rect::~Rect()
{
}

bool Rect::Contains(const Vec2& p) const
{
	return p.x >= m_p0.x && p.x <= m_p1.x && p.y >= m_p0.y && p.y <= m_p1.y;
}

void Rect::Normalise()
{
	if (m_p0.x > m_p1.x)
		std::swap(m_p0.x, m_p1.x);

	if (m_p0.y > m_p1.y)
		std::swap(m_p0.y, m_p1.y);
}

void Rect::GrowTo(const Vec2& p)
{
	m_p0.x = std::min(m_p0.x, p.x);
	m_p1.x = std::max(m_p1.x, p.x);
	m_p0.y = std::min(m_p0.y, p.y);
	m_p1.y = std::max(m_p1.y, p.y);
}

void Rect::Inflate(double x, double y)
{
	m_p0.x -= x;
	m_p0.y -= y;
	m_p1.x += x * 2;
	m_p1.y += y * 2;
}
