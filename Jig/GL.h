#pragma once

#include "VectorFwd.h"

namespace Jig
{
	namespace GL
	{
		void Translate(const Vec3& vec);
		void Rotate(double angle, const Vec3& vec);
		void Vertex(const Vec3& vec);
		void Normal(const Vec3& vec);
	}
}

