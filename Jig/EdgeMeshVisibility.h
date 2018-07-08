#pragma once

#include "EdgeMesh.h"

namespace Jig
{
	class EdgeMeshVisibility
	{
	public:
		using VisibleVec = std::vector<const EdgeMesh::Vert*>;
		
		class Data : public EdgeMesh::Data
		{
		public:
			Data(VisibleVec&& visible) : visible(visible) {}
			VisibleVec visible;
		};

		static const Data* GetData(const EdgeMesh::Vert* vert) { return static_cast<const Data*>(vert->GetData()); }
		static void Update(EdgeMesh& mesh);
	};
}
