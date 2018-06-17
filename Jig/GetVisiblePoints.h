#pragma once

#include "EdgeMesh.h"

#include <vector>

namespace Jig
{
	EdgeMesh::Vert::VisibleVec GetVisiblePoints(const EdgeMesh& mesh, const Vec2& point);
	bool IsVisible(const EdgeMesh& mesh, const Vec2 & point0, const Vec2 & point1);
}
