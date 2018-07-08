#include "EdgeMeshVisibility.h"
#include "GetVisiblePoints.h"

using namespace Jig;

void EdgeMeshVisibility::Update(EdgeMesh& mesh)
{
	for (auto& v : mesh.GetVerts())
		v->SetData(std::make_unique<Data>(Jig::GetVisiblePoints(mesh, *v)));
}
