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

		template <typename LineLoop1T, typename LineLoop2T>
		bool PolygonContainsPolygon(const LineLoop1T& outer, const LineLoop2T& inner)
		{
			for (auto& innerLine : inner)
			{
				if (!PointInPolygon(outer, innerLine.GetP0()))
					return false;

				for (auto& outerLine : outer)
					if (innerLine.Intersect(outerLine))
						return false;
			}
			return true;
		}
	}
}

