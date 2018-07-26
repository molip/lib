#pragma once

#include "EdgeMesh.h"

namespace Jig
{
	namespace EdgeMeshCommand { class AddFace; }
	using AddFaceCommandPtr = std::unique_ptr<EdgeMeshCommand::AddFace>;
		
	AddFaceCommandPtr EdgeMeshAddFace(EdgeMesh& edgeMesh, EdgeMesh::Vert& start, EdgeMesh::Vert& end, const PolyLine& polyline);
}

