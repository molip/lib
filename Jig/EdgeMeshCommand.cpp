#include "EdgeMeshCommand.h"
#include "Polygon.h"

using namespace Jig;
using namespace Jig::EdgeMeshCommand;

Base::~Base() = default;

Compound::Compound() = default;

Compound::Compound(Vec&& children) : m_children(std::move(children)) {}

void Compound::AddChild(std::unique_ptr<Base> child) 
{
	m_children.push_back(std::move(child));
}

void Compound::Do() 
{
	for (auto& child : m_children)
		child->Do();
}

void Compound::Undo()
{
	for (auto& child : Kernel::Reverse(m_children))
		child->Undo();
}



InsertVert::InsertVert(EdgeMesh& mesh, Jig::EdgeMesh::Edge& edge, const Jig::Vec2& pos) : m_mesh(mesh), m_pos(pos)
{
	//      1.new     1.old  
	// ---------* --------*
	// *--------- *-------- 
	// 0.new      0.old

	m_newVert = std::make_unique<EdgeMesh::Vert>(pos);

	m_items.emplace_back(&edge).newEdge = std::make_unique<EdgeMesh::Edge>();

	if (edge.twin)
	{
		m_items.emplace_back(edge.twin).newEdge = std::make_unique<EdgeMesh::Edge>(m_newVert.get());
		m_items[0].newEdge->SetTwin(*m_items[1].newEdge);
	}
}

void InsertVert::Do() 
{
	AssertFacesValid();

	m_items[0].oldEdge->prev->ConnectTo(*m_items[0].newEdge);
	m_items[0].newEdge->ConnectTo(*m_items[0].oldEdge);

	if (m_items.size() == 2)
	{
		m_items[1].newEdge->ConnectTo(*m_items[1].oldEdge->next);
		m_items[1].oldEdge->ConnectTo(*m_items[1].newEdge);
	}

	m_items[0].newEdge->vert = m_items[0].oldEdge->vert;
	m_items[0].oldEdge->vert = m_newVert.get();

	for (auto& item : m_items)
		item.oldEdge->face->PushEdge(std::move(item.newEdge));

	m_mesh.PushVert(std::move(m_newVert));

	AssertFacesValid();
}

void InsertVert::Undo()
{
	AssertFacesValid();

	m_newVert = m_mesh.PopVert();

	for (auto& item : Kernel::Reverse(m_items))
		item.newEdge = item.oldEdge->face->PopEdge();

	m_items[0].oldEdge->vert = m_items[0].newEdge->vert;

	m_items[0].newEdge->prev->ConnectTo(*m_items[0].oldEdge);

	if (m_items.size() == 2)
		m_items[1].oldEdge->ConnectTo(*m_items[1].newEdge->next);

	AssertFacesValid();
}

void InsertVert::AssertFacesValid() const
{
	for (auto& item : m_items)
		item.oldEdge->face->AssertValid();
}

AddOuterFace::AddOuterFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points) : m_mesh(mesh)
{
	KERNEL_ASSERT(!start.twin && !end.twin);

	m_face = std::make_unique<EdgeMesh::Face>();

	Polygon testPoly(points);
	for (auto& point : EdgeMesh::OuterPointLoop(end, *start.FindNextOuterEdge()))
		testPoly.push_back(point);

	const bool cw = testPoly.IsCW();

	auto& oldStart = cw ? start : end; 
	auto& oldEnd = cw ? end : start;

	Init(oldStart, oldEnd, cw ? points : points.GetReversed());
}

void AddOuterFace::Init(Jig::EdgeMesh::Edge& oldStart, Jig::EdgeMesh::Edge& oldEnd, const PolyLine& points)
{
	auto& edges = m_face->GetEdgesUnordered();

	auto addEdge = [&](auto* vert)
	{
		auto edge = std::make_unique<EdgeMesh::Edge>(vert);
		if (!edges.empty())
			edge->ConnectTo(*edges.back());

		m_face->PushEdge(std::move(edge));
	};

	for (auto& edge : EdgeMesh::OuterEdgeLoop(oldStart, oldEnd))
	{
		addEdge(edge.next->vert);
		m_oldEdges.push_back(&edge);
	}

	for (auto& point : Kernel::Reverse(points))
	{
		m_newVerts.push_back(std::make_unique<Jig::EdgeMesh::Vert>(point));
		addEdge(m_newVerts.back().get());
	}

	addEdge(oldStart.vert);
	edges.front()->ConnectTo(*edges.back());

	m_face->AssertValid();
}

void AddOuterFace::Do()
{
	for (auto& vert : m_newVerts)
		m_mesh.PushVert(std::move(vert));

	auto& edges = m_face->GetEdgesUnordered();

	size_t i = 0;
	for (auto& edge : m_oldEdges)
		edge->SetTwin(*edges[i++]);

	m_face->AssertValid();

	m_mesh.PushFace(std::move(m_face));
}

void AddOuterFace::Undo()
{
	m_face = m_mesh.PopFace();
	m_face->AssertValid();

	auto& edges = m_face->GetEdgesUnordered();

	size_t i = 0;
	for (auto& edge : m_oldEdges)
		edge->twin = nullptr;

	for (auto& vert : Kernel::Reverse(m_newVerts))
		vert = m_mesh.PopVert();
}



SplitFace::SplitFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points) : m_mesh(mesh), m_start(start), m_end(end)
{
	KERNEL_ASSERT(&start != &end && start.face == end.face);

	CreateNewEdgesForOldFace(points);
	CreateNewFace();
}

void SplitFace::CreateNewEdgesForOldFace(const PolyLine& points)
{
	m_newEdges.push_back(std::make_unique<EdgeMesh::Edge>(m_start.vert));

	for (auto& point : points)
	{
		m_newVerts.push_back(std::make_unique<Jig::EdgeMesh::Vert>(point));

		auto edge = std::make_unique<EdgeMesh::Edge>(m_newVerts.back().get());
		m_newEdges.back()->ConnectTo(*edge);
		m_newEdges.push_back(std::move(edge));
	}

	m_newEdges.front()->prev = m_start.prev;
	m_newEdges.back()->next = &m_end;
}

void SplitFace::CreateNewFace()
{
	// Just add twins. The outer edges still belong to the old face. 

	m_face = std::make_unique<EdgeMesh::Face>();
	auto& edges = m_face->GetEdgesUnordered();

	m_face->PushEdge(std::make_unique<EdgeMesh::Edge>(m_end.vert));

	for (auto& vert : Kernel::Reverse(m_newVerts))
	{
		m_face->PushEdge(std::make_unique<EdgeMesh::Edge>(vert.get()));
		edges[edges.size() - 2]->ConnectTo(*edges.back());
	}
	
	int i = 0;
	for (auto& edge : edges)
		edge->SetTwin(**(m_newEdges.rbegin() + i++));

	edges.front()->prev = m_end.prev;
	edges.back()->next = &m_start;
}

void SplitFace::Do()
{
	m_end.face->AssertValid();

	// Move outer edges to new face.
	for (auto& edge : EdgeMesh::EdgeLoop(m_start, m_end))
	{
		auto[edgePtr, index] = m_end.face->RemoveEdge(edge);
		m_face->PushEdge(std::move(edgePtr));
		m_oldEdgePositions.push_back(index);
	}

	// Connect edges.
	m_start.prev->next = m_newEdges.front().get();
	m_end.prev->next = &m_face->GetEdge();
	m_start.prev = m_newEdges.front()->twin;
	m_end.prev = m_newEdges.back().get();

	// Push everything.
	for (auto& edge : m_newEdges)
		m_end.face->PushEdge(std::move(edge));

	for (auto& vert : m_newVerts)
		m_mesh.PushVert(std::move(vert));

	m_face->AssertValid();
	m_end.face->AssertValid();

	m_mesh.PushFace(std::move(m_face));
}

void SplitFace::Undo()
{
	// Pop everything.
	m_face = m_mesh.PopFace();

	m_face->AssertValid();
	m_end.face->AssertValid();

	for (auto& vert : Kernel::Reverse(m_newVerts))
		vert = m_mesh.PopVert();

	for (auto& edge : Kernel::Reverse(m_newEdges))
		edge = m_end.face->PopEdge();

	// Reconnect edges.
	m_face->GetEdge().prev->ConnectTo(m_end);
	m_newEdges.front()->prev->ConnectTo(m_start);

	// Move outer edges back to old face.
	for (size_t index : Kernel::Reverse(m_oldEdgePositions))
		m_end.face->InsertEdge(m_face->PopEdge(), index);

	m_oldEdgePositions.clear();

	m_end.face->AssertValid();
}
