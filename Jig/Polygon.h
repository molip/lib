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

		Polygon(const PolyLine& rhs) : PolyLine(rhs)
		{
			SetClosed(true);
		}
	};
}