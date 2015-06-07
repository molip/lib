#pragma once

#include "Line2.h"
#include "Rect.h"

#include <vector>

namespace Jig
{
	namespace Geometry
	{
		// http://geomalgorithms.com/a03-_inclusion.html
		template <typename PointPairLoopT>
		bool PointInPolygon(const PointPairLoopT& pointPairLoop, const Vec2& point)
		{
			bool inside = false;
			const Line2 test = Line2::MakeHorizontal(point.y);
			auto IsOnTestLine = [&](const Vec2& p) { return p.x > point.x && ::fabs(p.y - point.y) < Epsilon; };

			for (auto& pair : pointPairLoop)
			{
				if (point == pair.first)
					return true;

				if (::fabs(pair.first.y - pair.second.y) > Epsilon) // Not horizontal.
				{
					if (pair.first.y < pair.second.y) // Y increasing.
						std::swap(pair.first, pair.second);

					if (!IsOnTestLine(pair.second))
					{
						Vec2 ip;
						if (IsOnTestLine(pair.first) || (Line2::MakeFinite(pair.first, pair.second).Intersect(test, &ip) && ip.x > point.x))
							inside = !inside;
					}
				}
			}
			return inside;
		}

		template <typename PointPairLoop1T, typename PointPairLoop2T>
		bool PolygonContainsPolygon(const PointPairLoop1T& outer, const PointPairLoop2T& inner)
		{
			for (auto& innerPair : inner)
			{
				if (!PointInPolygon(outer, innerPair.first))
					return false;

				const Line2 innerLine = Line2::MakeFinite(innerPair.first, innerPair.second);

				for (auto& outerPair : outer)
					if (innerLine.Intersect(Line2::MakeFinite(outerPair.first, outerPair.second)))
						return false;
			}
			return true;
		}

		template <typename LineLoopT>
		Vec2 GetClosestPoint(const LineLoopT& lineLoop, const Vec2& point, double* closestDist = nullptr)
		{
			Vec2 closest;
			double minDist = std::numeric_limits<double>::max();
			for (auto& edge : lineLoop)
			{
				double dist = 0;
				Vec2 intersection;
				if (!edge.PerpIntersect(point, &dist, &intersection))
				{
					intersection = edge.GetP0();
					dist = Line2::MakeFinite(point, intersection).Length();
				}

				if (dist < minDist)
				{
					closest = intersection;
					minDist = dist;
				}
			}
			if (closestDist)
				*closestDist = minDist;
			return closest;
		}

		template <typename PointsT>
		Rect Geometry::GetBBox(const PointsT& points)
		{
			if (!(points.begin() != points.end()))
				return Rect();

			Rect r(*points.begin());
			for (const Vec2& p : points)
				r.GrowTo(p);

			return r;
		}
	}
}

