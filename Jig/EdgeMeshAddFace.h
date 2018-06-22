#pragma once

#include "EdgeMesh.h"

namespace Jig
{
	bool EdgeMeshAddFace(EdgeMesh& edgeMesh, EdgeMesh::Vert& start, EdgeMesh::Vert& end, const PolyLine& polyline);
}

