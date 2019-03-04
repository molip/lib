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

bool Compound::CanDo() const 
{
	for (auto& child : m_children)
		if (!child->CanDo())
			return false;

	return true; 
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



InsertVerts::InsertVerts(EdgeMesh& mesh, EdgeMesh::Edge& edge, const Vec2& pos) : InsertVerts(mesh, edge, { pos })
{
}

InsertVerts::InsertVerts(EdgeMesh& mesh, EdgeMesh::Edge& edge, std::initializer_list<Vec2> positions) : m_mesh(mesh)
{
	//     1.new[1]   1.new[0]     1.old  
	//   ---------* ---------* --------*
	// *--------- *--------- *--------- 
	// 0.old      0.new[0]	 0.new[1]

	for (auto& pos : positions)
	{
		m_newVerts.emplace_back(std::make_unique<EdgeMesh::Vert>(pos));
	}


	auto add = [&](EdgeMesh::Edge& edge, const auto& verts)
	{
		auto* prev = &edge;
		auto& item = m_items.emplace_back(&edge);
		for (auto& vert : verts)
		{
			auto& newEdge = *item.newEdges.emplace_back(std::make_unique<EdgeMesh::Edge>(vert.get()));
			newEdge.prev = prev;
			if (prev != &edge) // Don't modify edge yet - wait until Do(). 
				prev->next = &newEdge;
			
			prev = &newEdge;
		}

		prev->next = edge.next;
	};

	add(edge, m_newVerts);

	if (edge.twin)
	{
		add(*edge.twin, Kernel::Reverse(m_newVerts));

		// Twin the end edges.
		m_items[0].newEdges.back()->twin = m_items[1].oldEdge;
		m_items[1].newEdges.back()->twin = m_items[0].oldEdge;

		// Twin the middle edges.
		const size_t n = m_items[0].newEdges.size() - 1;
		for (size_t i = 0; i < n; ++i)
			m_items[0].newEdges[i]->SetTwin(*m_items[1].newEdges[n - i - 1]);
	}
}

void InsertVerts::Do() 
{
	AssertFacesValid();

	if (m_items.size() == 2)
	{
		m_items[0].oldEdge->twin = m_items[1].newEdges.back().get();
		m_items[1].oldEdge->twin = m_items[0].newEdges.back().get();
	}

	for (auto& item : m_items)
	{
		item.oldEdge->next->prev = item.newEdges.back().get();
		item.oldEdge->next = item.newEdges.front().get();

		for (auto& edge : item.newEdges)
			item.oldEdge->face->PushEdge(std::move(edge));
	}

	for (auto& vert : m_newVerts)
		m_mesh.PushVert(std::move(vert));

	AssertFacesValid();
}

void InsertVerts::Undo()
{
	AssertFacesValid();

	for (auto& vert : Kernel::Reverse(m_newVerts))
		vert = m_mesh.PopVert();

	for (auto& item : Kernel::Reverse(m_items))
	{
		for (auto& edge : Kernel::Reverse(item.newEdges))
			edge = item.oldEdge->face->PopEdge();

		item.oldEdge->ConnectTo(*item.newEdges.back()->next);
	}

	if (m_items.size() == 2)
		m_items[0].oldEdge->SetTwin(*m_items[1].oldEdge);

	AssertFacesValid();
}

void InsertVerts::AssertFacesValid() const
{
	for (auto& item : m_items)
		item.oldEdge->face->AssertValid();
}



DeleteVert::DeleteVert(EdgeMesh& mesh, EdgeMesh::Vert& vert) : m_mesh(mesh), m_vert(vert)
{
	auto* anyEdge = m_mesh.FindEdgeWithVert(m_vert);
	KERNEL_VERIFY(anyEdge);

	m_startEdge = &anyEdge->GetFirstShared();
}

bool DeleteVert::CanDo() const
{
	auto isOnlyTwin = [](const EdgeMesh::Edge& edge)
	{
		if (!edge.GetTwinFace())
			return false;

		for (auto& e : EdgeMesh::ConstEdgeLoop(*edge.next, edge))
			if (e.GetTwinFace() == edge.GetTwinFace())
				return false;
		
		return true;
	};

	for (auto& edge : m_startEdge->GetSharedEdges())
	{
		if (edge.face->GetEdgeCount() < 4)
			return false;
	
		if (isOnlyTwin(edge))
			return false;
	}

	return true;
}

void DeleteVert::Do()
{
	for (auto& edge : m_startEdge->GetSharedEdges())
	{
		auto[edgeptr, index] = edge.face->RemoveEdge(edge);
		auto& item = m_items.emplace_back(std::move(edgeptr), index);

		item.oldPrev = edge.prev;
		item.oldPrevTwin = edge.prev->twin;
	}

	if (m_startEdge->twin && m_startEdge->twin->next == m_startEdge->prev->twin)
	{
		// Simple pair of twinned edges - re-twin.
		KERNEL_VERIFY(m_items.size() == 2);
		m_startEdge->prev->SetTwin(*m_startEdge->twin);
	}
	else 
	{
		// De-twin for all other cases. 
		for (auto& item : m_items)
		{
			auto& edge = *item.oldEdge;
			KERNEL_VERIFY(!edge.prev->twin || edge.prev->twin->vert == &m_vert);
			edge.prev->twin = nullptr;
			
			if (edge.twin)
				edge.twin->twin = nullptr;
		}
	}

	for (auto& item : m_items)
		item.oldEdge->prev->ConnectTo(*item.oldEdge->next);

	auto[vert, index] = m_mesh.RemoveVert(m_vert);
	m_oldVert = std::move(vert);
	m_oldVertIndex = index;

	for (auto& item : m_items)
		item.oldPrev->face->AssertValid();
}

void DeleteVert::Undo()
{
	m_mesh.InsertVert(std::move(m_oldVert), m_oldVertIndex);

	for (auto& item : Kernel::Reverse(m_items))
	{
		item.oldPrev->ConnectTo(*item.oldEdge);
		item.oldEdge->ConnectTo(*item.oldEdge->next);
	}

	for (auto& item : Kernel::Reverse(m_items))
	{
		if (item.oldEdge->twin)
			item.oldEdge->twin->twin = item.oldEdge.get();

		if (item.oldPrevTwin)
			item.oldPrev->SetTwin(*item.oldPrevTwin);
	
		item.oldPrev->face->InsertEdge(std::move(item.oldEdge), item.oldIndex);
	}

	for (auto& item : m_items)
		item.oldPrev->face->AssertValid();

	m_items.clear();
}



AddFace::AddFace(EdgeMesh& mesh) : m_mesh(mesh)
{
	m_face = std::make_unique<EdgeMesh::Face>();
}



AddOuterFace::AddOuterFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points) : AddFace(mesh)
{
	KERNEL_ASSERT(!start.twin && !end.twin);

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



SplitFace::SplitFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points) : AddFace(mesh), m_start(start), m_end(end)
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



MoveVert::MoveVert(EdgeMesh::Vert& vert, const Vec2 & pos) : m_vert(vert), m_pos(pos)
{
}

void MoveVert::Do()
{
	std::swap(m_vert.x, m_pos.x);
	std::swap(m_vert.y, m_pos.y);
}

void MoveVert::Undo()
{
	Do();
}

MergeFace::MergeFace(EdgeMesh& mesh, EdgeMesh::Edge& edge) : m_mesh(mesh), m_edge(edge)
{
	KERNEL_ASSERT(edge.twin);

	for (auto* e = &edge; e->GetTwinFace() == edge.GetTwinFace(); e = e->prev)
		m_first = e;

	for (auto* e = &edge; e->GetTwinFace() == edge.GetTwinFace(); e = e->next)
		m_last = e;
}

bool MergeFace::CanDo() const
{
	return true;
}

void MergeFace::Do()
{
	auto& face = *m_first->face;
	auto& other = *m_first->twin->face;

	face.AssertValid();
	other.AssertValid();

	for (auto& e : EdgeMesh::EdgeLoop(*m_first, *m_last->next))
	{
		m_deleted.emplace_back(face.RemoveEdge(e), other.RemoveEdge(*e.twin));
		m_deletedEdges.push_back(&e);
		m_deletedEdges.push_back(e.twin);
		if (&e != m_first)
		{
			m_oldVerts.push_back(m_mesh.RemoveVert(*e.vert));
			m_deletedVerts.push_back(e.vert);
		}
	}

	for (auto& e : EdgeMesh::EdgeLoop(*m_last->next, *m_first))
	{
		auto[edgePtr, index] = face.RemoveEdge(e);
		other.PushEdge(std::move(edgePtr));
		m_adopted.push_back(index);
	}

	m_first->prev->ConnectTo(*m_first->twin->next);
	m_last->twin->prev->ConnectTo(*m_last->next);
				
	m_oldFace = m_mesh.RemoveFace(face);

	other.AssertValid();
}

void MergeFace::Undo()
{
	auto& face = *m_first->face;
	auto& other = *m_first->twin->face;

	other.AssertValid();

	m_mesh.InsertFace(std::move(m_oldFace.first), m_oldFace.second);

	m_first->prev->next = m_first;
	m_first->twin->next->prev = m_first->twin;
	m_last->next->prev = m_last;
	m_last->twin->prev->next = m_last->twin;

	for (size_t index : Kernel::Reverse(m_adopted))
		face.InsertEdge(other.PopEdge(), index);

	for (auto& pair : Kernel::Reverse(m_deleted))
	{
		face.InsertEdge(std::move(pair.first.first), pair.first.second);
		other.InsertEdge(std::move(pair.second.first), pair.second.second);
	}

	for (auto& item : Kernel::Reverse(m_oldVerts))
		m_mesh.InsertVert(std::move(item.first), item.second);

	m_adopted.clear();
	m_deleted.clear();
	m_oldVerts.clear();
	m_deletedVerts.clear();
	m_deletedEdges.clear();

	face.AssertValid();
	other.AssertValid();
}

DeleteFace::DeleteFace(EdgeMesh& mesh, EdgeMesh::Face& face) : m_mesh(mesh), m_face(face)
{
}

bool DeleteFace::CanDo() const
{
	return true;
}

void DeleteFace::Do()
{
	m_face.AssertValid();

	for (auto& e : m_face.GetEdges())
	{
		if (!e.twin && !e.prev->twin)
		{
			m_oldVerts.push_back(m_mesh.RemoveVert(*e.vert));
			m_deletedVerts.push_back(e.vert);
		}

		m_oldTwins.push_back(e.twin);
		if (e.twin)
			e.twin->twin = nullptr;
	}

	m_oldFace = m_mesh.RemoveFace(m_face);
}

void DeleteFace::Undo()
{
	m_mesh.InsertFace(std::move(m_oldFace.first), m_oldFace.second);

	int index = 0;
	for (auto& e : m_face.GetEdges())
	{
		if (m_oldTwins[index])
			m_oldTwins[index]->twin = &e;
		++index;
	}
	
	for (auto& item : Kernel::Reverse(m_oldVerts))
		m_mesh.InsertVert(std::move(item.first), item.second);

	m_oldTwins.clear();
	m_oldVerts.clear();
	m_deletedVerts.clear();

	m_face.AssertValid();
}
