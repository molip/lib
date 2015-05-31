#pragma once

#include "EdgeMesh.h"

#include <vector>

namespace Jig
{
	std::vector<Vec2> GetVisiblePoints(const EdgeMesh& mesh, const Vec2& point);
}
