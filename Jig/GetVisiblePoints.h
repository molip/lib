#pragma once

#include "EdgeMesh.h"

#include <vector>

namespace Jig
{
	std::vector<const EdgeMesh::Vert*> GetVisiblePoints(const EdgeMesh& mesh, const Vec2& point);
	bool IsVisible(const EdgeMesh& mesh, const Vec2 & point0, const Vec2 & point1);
}
