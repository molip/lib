#include "EdgeMeshInternalEdges.h"
#include "EdgeMesh.h"

#include "libKernel/Debug.h"

#include <map>
#include <set>

using namespace Jig;

EdgeMeshInternalEdges::EdgeMeshInternalEdges(const EdgeMesh& edgeMesh)
{
	using Edge = EdgeMesh::Edge;
	using Vert = EdgeMesh::Vert;

	struct Target
	{
		const Vert* vert{};
		bool outer{};
	};

	using Map = std::map<const Vert*, std::vector<Target>>;
	Map map;

	for (auto& face : edgeMesh.GetFaces())
		for (auto& edge : face->GetEdges())
			if (edge.twin)
			{
				auto[it, ok] = map.emplace(edge.vert, Map::mapped_type());
				if (ok)
				{
					auto& targets = it->second;

					const Edge* outer = edge.FindSharedOuterEdge();
					const Edge* first = outer ? outer : &edge;
					const Edge* add = first;
					do
					{
						targets.push_back(Target{ add->next->vert, add->twin == nullptr });
						const Edge* next = add->GetNextShared();
						if (!next) // add->prev is outer.
							targets.push_back(Target{ add->prev->vert, true });
						
						add = next;
					} while (add && add != first);
				}
			}

	std::set<const Vert*> done;

	for (auto[vert, targets] : map)
	{
		//Kernel::Debug::Trace << vert << std::endl;
		for (auto& target : targets)
		{
			//Kernel::Debug::Trace << "\t" << target.vert << (target.outer ? " outer" : "") << std::endl;
			if (!target.outer && done.count(target.vert) == 0)
				m_lines.push_back(Line2::MakeFinite(*vert, *target.vert));
		}

		done.insert(vert);
	}
}
