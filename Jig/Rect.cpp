#include "Rect.h"

using namespace Jig;

Rect::Rect()
{
}

Rect::Rect(const Vec2& p0, const Vec2& p1) : m_p0(p0), m_p1(p1)
{
}

Rect::~Rect()
{
}

void Rect::Normalise()
{
	if (m_p0.x > m_p1.x)
		std::swap(m_p0.x, m_p1.x);

	if (m_p0.y > m_p1.y)
		std::swap(m_p0.y, m_p1.y);
}