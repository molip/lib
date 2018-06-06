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
}