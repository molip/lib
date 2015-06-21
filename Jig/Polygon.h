#pragma once

#include "PolyLine.h"

namespace Jig
{
	class Polygon : public PolyLine
	{
	public:
		Polygon() 
		{
			SetClosed(true); 
		}
	};

	using PolyPolygon = std::vector<Polygon>;
}