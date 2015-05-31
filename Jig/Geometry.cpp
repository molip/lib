#include "Geometry.h"

Jig::Rect Jig::Geometry::GetBBox(const std::vector<Vec2>& points)
{
	if (points.empty())
		return Rect();

	Rect r(points.front());
	for (const Vec2& p : points)
		r.GrowTo(p);

	return r;
}

