#include "EdgeMeshProxy.h"

#include "libKernel/Serial.h"

using namespace Jig;

EdgeMeshProxy::EdgeMeshProxy(const EdgeMesh& mesh)
{
	SaveContext ctx(mesh);
	for (auto& vert : mesh.GetVerts())
		m_verts.push_back(Vert(*vert));

	for (auto& face : mesh.GetFaces())
		m_faces.push_back(Face(*face, ctx));
}

void EdgeMeshProxy::Save(Serial::SaveNode& node) const
{
	node.SaveCntr("verts", m_verts, Serial::ClassSaver());
	node.SaveCntr("faces", m_faces, Serial::ClassSaver());
}

void EdgeMeshProxy::Load(const Serial::LoadNode& node, EdgeMesh& mesh)
{
	KERNEL_ASSERT(mesh.GetVerts().empty());

	node.LoadCntr("verts", m_verts, Serial::ClassLoader());
	node.LoadCntr("faces", m_faces, Serial::ClassLoader());

	std::vector<EdgeMesh::Edge*> newEdges;

	for (auto& vert : m_verts)
		mesh.AddVert(vert.CreateVert());

	for (auto& face : m_faces)
	{
		auto newFace = std::make_unique<EdgeMesh::Face>();

		for (auto& edge : face.m_edges)
			newEdges.push_back(&newFace->AddEdge(std::make_unique<EdgeMesh::Edge>()));

		mesh.AddFace(std::move(newFace));
	}

	int index = 0;
	for (auto& face : m_faces)
	{
		for (auto& edge : face.m_edges)
		{
			auto& newEdge = *newEdges[index++];
			newEdge.vert = mesh.GetVerts()[edge.m_vert].get();
			newEdge.prev = newEdges[edge.m_prev];
			newEdge.next = newEdges[edge.m_next];
			newEdge.twin = edge.m_twin >= 0 ? newEdges[edge.m_twin] : nullptr;
		}
	}
}



EdgeMeshProxy::SaveContext::SaveContext(const EdgeMesh& mesh)
{
	for (auto& vert : mesh.GetVerts())
		vertMap[vert.get()] = (int)vertMap.size();

	for (auto& face : mesh.GetFaces())
		for (auto& edge : face->GetEdges())
			edgeMap[&edge] = (int)edgeMap.size();
}



EdgeMeshProxy::DataOwner::DataOwner(const EdgeMesh::DataOwner& obj) : m_data(const_cast<EdgeMesh::Data*>(obj.GetData()))
{
}

void EdgeMeshProxy::DataOwner::Save(Serial::SaveNode& node) const
{
	if (m_data)
		node.SaveObject("data", m_data);
}

void EdgeMeshProxy::DataOwner::Load(const Serial::LoadNode& node)
{
	node.LoadObject("data", m_data);
}



EdgeMeshProxy::Vert::Vert(const EdgeMesh::Vert& obj) : DataOwner(obj), m_position(obj) {}

void EdgeMeshProxy::Vert::Save(Serial::SaveNode& node) const
{
	node.SaveType("pos", m_position);
	__super::Save(node);
}

void EdgeMeshProxy::Vert::Load(const Serial::LoadNode& node)
{
	node.LoadType("pos", m_position);
	__super::Load(node);
}

EdgeMesh::VertPtr EdgeMeshProxy::Vert::CreateVert()
{
	auto vert = std::make_unique<EdgeMesh::Vert>(m_position);
	vert->SetData(EdgeMesh::DataPtr(m_data));
	m_data = nullptr;
	return vert;
}



EdgeMeshProxy::Edge::Edge(const EdgeMesh::Edge& obj, SaveContext& ctx) : DataOwner(obj)
{
	KERNEL_ASSERT(obj.vert && obj.prev && obj.next);

	m_vert = ctx.vertMap[obj.vert];
	m_prev = ctx.edgeMap[obj.prev];
	m_next = ctx.edgeMap[obj.next];
	m_twin = obj.twin ? ctx.edgeMap[obj.twin] : -1;
}

void EdgeMeshProxy::Edge::Save(Serial::SaveNode& node) const
{
	node.SaveType("vert", m_vert);
	node.SaveType("prev", m_prev);
	node.SaveType("next", m_next);
	node.SaveType("twin", m_twin);
	__super::Save(node);
}

void EdgeMeshProxy::Edge::Load(const Serial::LoadNode& node)
{
	node.LoadType("vert", m_vert);
	node.LoadType("prev", m_prev);
	node.LoadType("next", m_next);
	node.LoadType("twin", m_twin);
	__super::Load(node);
}



EdgeMeshProxy::Face::Face(const EdgeMesh::Face& obj, SaveContext& ctx) : DataOwner(obj)
{
	for (auto& edge : obj.GetEdges())
		m_edges.push_back(Edge(edge, ctx));
}

void EdgeMeshProxy::Face::Save(Serial::SaveNode& node) const
{
	node.SaveCntr("edges", m_edges, Serial::ClassSaver());
	__super::Save(node);
}

void EdgeMeshProxy::Face::Load(const Serial::LoadNode& node)
{
	node.LoadCntr("edges", m_edges, Serial::ClassLoader());
	__super::Load(node);
}
