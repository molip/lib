#pragma once

#include "Line2.h"

namespace Jig
{
	namespace Geometry
	{
		// http://geomalgorithms.com/a03-_inclusion.html
		template <typename LineLoopT>
		bool PointInPolygon(const LineLoopT& lineLoop, const Vec2& point) 
		{
			bool inside = false;
			const Line2 test = Line2::MakeHorizontal(point.y);
			auto IsOnTestLine = [&](const Vec2& p) { return p.x > point.x && ::fabs(p.y - point.y) < Epsilon; };

			for (auto& edge : lineLoop)
			{
				if (!edge.IsHorizontal())
				{
					if (edge.GetP0().y < edge.GetP1().y) // Y increasing.
						edge.SwapPoints();

					if (!IsOnTestLine(edge.GetP1()))
					{
						Vec2 ip;
						if (IsOnTestLine(edge.GetP0()) || (edge.Intersect(test, &ip) && ip.x > point.x))
							inside = !inside;
					}
				}
			}
			return inside;
		}

	}
}

