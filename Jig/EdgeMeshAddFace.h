#pragma once

#include "EdgeMesh.h"
#include "EdgeMesh.h"

namespace Jig
{
	namespace EdgeMeshCommand { class Base; }
	using EdgeMeshCommandPtr = std::unique_ptr<EdgeMeshCommand::Base>;
		
	EdgeMeshCommandPtr EdgeMeshAddFace(EdgeMesh& edgeMesh, EdgeMesh::Vert& start, EdgeMesh::Vert& end, const PolyLine& polyline);
}

