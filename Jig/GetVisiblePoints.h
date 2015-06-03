#pragma once

#include "EdgeMesh.h"

#include <vector>

namespace Jig
{
	EdgeMesh::VertPtrVec GetVisiblePoints(const EdgeMesh& mesh, const Vec2& point);
}
