#include "EdgeMesh.h"

#include "Geometry.h"
#include "Polygon.h"

#include "libKernel/Debug.h"
#include "libKernel/MinFinder.h"

#include <cassert>

using namespace Jig;
using namespace Kernel;

EdgeMesh::EdgeMesh(EdgeMesh&& rhs) : m_faces(std::move(rhs.m_faces)), m_verts(std::move(rhs.m_verts))
{
}

Jig::EdgeMesh::EdgeMesh(std::vector<VertPtr>&& verts) : m_verts(std::move(verts))
{
}

void EdgeMesh::Save(Kernel::Serial::SaveNode& node) const
{
	Kernel::Serial::SaveContext ctx(node);

	node.SaveCntr("verts", m_verts, Serial::ClassPtrSaver());
	node.SaveCntr("faces", m_faces, Serial::ClassPtrSaver());
}

void EdgeMesh::Load(const Kernel::Serial::LoadNode& node)
{
	Kernel::Serial::LoadContext ctx(node);

	node.LoadCntr("verts", m_verts, Serial::ClassPtrLoader());
	node.LoadCntr("faces", m_faces, Serial::ClassPtrLoader());

	ctx.ResolveRefs();

	for (auto& face : m_faces)
	{
		face->Dump();
		face->AssertValid();
	}
}

void Jig::EdgeMesh::operator=(EdgeMesh && rhs)
{
	m_faces = std::move(rhs.m_faces);
	m_verts = std::move(rhs.m_verts);
}

EdgeMesh::Vert& EdgeMesh::AddVert(const Vec2& point)
{
	m_verts.push_back(std::make_unique<Vert>(point));
	return *m_verts.back();
}

EdgeMesh::Face& EdgeMesh::PushFace(FacePtr face)
{
	m_faces.push_back(std::move(face));
	m_faces.back()->AssertValid();
	return *m_faces.back();
}

EdgeMesh::FacePtr EdgeMesh::PopFace()
{
	EdgeMesh::FacePtr face = std::move(m_faces.back());
	m_faces.pop_back();
	return face;
}

EdgeMesh::Vert& EdgeMesh::PushVert(EdgeMesh::VertPtr vert)
{
	m_verts.push_back(std::move(vert));
	return *m_verts.back();
}

EdgeMesh::VertPtr EdgeMesh::PopVert()
{
	EdgeMesh::VertPtr vert = std::move(m_verts.back());
	m_verts.pop_back();
	return vert;
}

void EdgeMesh::DeleteFace(Face& face)
{
	auto it = std::find_if(m_faces.begin(), m_faces.end(), [&](const FacePtr& f) { return f.get() == &face; });
	KERNEL_ASSERT(it != m_faces.end());
	m_faces.erase(it);
}

void EdgeMesh::DissolveEdge(Edge& edge)
{
	if (Face* merged = edge.face->DissolveEdge(edge, nullptr))
		DeleteFace(*merged);
}

void EdgeMesh::DissolveRedundantEdges()
{
	while (true)
	{
		bool changed = false;
		for (auto& face : m_faces)
			if (changed = DissolveRedundantEdges(*face))
				break;
		
		if (!changed)
			break;
	}
}

bool EdgeMesh::DissolveRedundantEdges(Face& face)
{
	for (auto& edge : face.GetEdges())
		if (edge.IsRedundant())
		{
			DissolveEdge(edge);
			return true;
		}

	return false;
}

bool EdgeMesh::Contains(const Polygon& poly) const
{
	return false;
}

void EdgeMesh::Clear() 
{
	m_faces.clear();
	m_verts.clear();
}

const EdgeMesh::Face* EdgeMesh::HitTest(const Vec2& point) const
{
	return m_quadTree.HitTest(point);
}

const EdgeMesh::Vert* EdgeMesh::FindNearestVert(const Vec2& point, double tolerance) const
{
	Jig::Rect bbox = m_bbox;
	if (tolerance > 0)
		bbox.Inflate(tolerance, tolerance);
	if (!bbox.Contains(point))
		return {};

	MinFinder<const Vert*> minFinder;
	if (tolerance)
		minFinder.SetThreshold(tolerance * tolerance);

	for (const auto& vert : m_verts)
		minFinder.Try(vert.get(), Jig::Vec2(point - *vert).GetLengthSquared());

	return minFinder.GetObject();
}

EdgeMesh::Edge* EdgeMesh::FindOuterEdge()
{
	for (auto& face : m_faces)
		if (Edge* edge = face->FindOuterEdge())
			return edge;

	return nullptr;
}

EdgeMesh::Edge* EdgeMesh::FindOuterEdgeWithVert(const Vert& vert)
{
	for (auto& edge : GetOuterEdges())
		if (edge.vert == &vert)
			return &edge;
	
	return nullptr;
}

Polygon EdgeMesh::GetOuterPolygon() const
{
	Polygon poly;
	for (auto& edge : GetOuterEdges())
		poly.push_back(*edge.vert);

	return poly;
}

void EdgeMesh::Update()
{
	RectGrower grower;

	for (auto& face : m_faces)
	{
		face->Update();
		grower.Add(face->GetBBox());
	}
	m_bbox = grower.GetRect();

	m_quadTree.Reset(m_bbox);

	for (auto& face : m_faces)
		m_quadTree.Insert(face.get());
}

void EdgeMesh::Dump() const
{
	Debug::Trace << "EdgeMesh " << std::hex << this << std::endl;

	for (auto& face : m_faces)
		face->Dump();

	Debug::Trace << std::endl;
}
//-----------------------------------------------------------------------------

EdgeMesh::Face::Face(Edge& edgeLoopToAdopt)
{
	AdoptEdgeLoop(edgeLoopToAdopt);
}

void EdgeMesh::Face::Save(Kernel::Serial::SaveNode& node) const
{
	__super::Save(node);
	node.SaveCntr("edges", m_edges, Kernel::Serial::ClassPtrSaver());
}

void EdgeMesh::Face::Load(const Kernel::Serial::LoadNode& node)
{
	__super::Load(node);
	node.LoadCntr("edges", m_edges, Kernel::Serial::ClassPtrLoader());

	for (auto& edge : m_edges)
		edge->face = this;
}

Polygon EdgeMesh::Face::GetPolygon() const
{
	Polygon poly;
	for (auto& edge : GetEdges())
		poly.push_back(*edge.vert);
	return poly;
}

EdgeMesh::Edge& EdgeMesh::Face::AddEdge(const Vert* vert)
{
	m_edges.push_back(std::make_unique<Edge>(vert, this));
	return *m_edges.back();
}

EdgeMesh::Edge& EdgeMesh::Face::PushEdge(EdgeMesh::EdgePtr edge)
{
	edge->face = this;
	m_edges.push_back(std::move(edge));
	return *m_edges.back();
}

EdgeMesh::EdgePtr EdgeMesh::Face::PopEdge()
{
	EdgeMesh::EdgePtr edge = std::move(m_edges.back());
	m_edges.pop_back();
	return edge;
}

std::pair<EdgeMesh::EdgePtr, size_t> EdgeMesh::Face::RemoveEdge(Edge& edge)
{
	const auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](auto& item) { return item.get() == &edge; });
	
	if (it == m_edges.end())
		return {};

	auto edgePtr = std::move(*it);
	edgePtr->face = nullptr;
	size_t index = it - m_edges.begin();
	m_edges.erase(it);

	return { std::move(edgePtr), index };
}

void EdgeMesh::Face::InsertEdge(EdgePtr edge, size_t index)
{
	edge->face = this;
	m_edges.insert(m_edges.begin() + index, std::move(edge));
}

EdgeMesh::Edge& EdgeMesh::Face::AddAndConnectEdge(const Vert* vert, Edge* after)
{
	Edge& e = AddEdge(vert);
		
	if (m_edges.size() == 1)
	{
		e.next = e.prev = &e;
		return e;
	}

	if (!after)
		after = (m_edges.end() - 2)->get(); // Caution - m_edges might be unordered! 
			
	e.ConnectTo(*after->next);
	after->ConnectTo(e);

	return e;
}

EdgeMesh::Edge* EdgeMesh::Face::FindOuterEdge() 
{
	for (auto& edge : EdgeLoop(GetEdge()))
		if (!edge.twin)
			return &edge;

	return nullptr;
}

EdgeMesh::Edge* EdgeMesh::Face::FindEdgeWithVert(const Vert& vert) 
{
	for (auto& edge : EdgeLoop(GetEdge()))
		if (edge.vert == &vert)
			return &edge;
	
	return nullptr;
}

std::vector<EdgeMesh::EdgePtr>::iterator EdgeMesh::Face::FindEdge(Edge& edge)
{
	auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgePtr& e) { return e.get() == &edge; });
	KERNEL_ASSERT(it != m_edges.end());
	return it;
}

void EdgeMesh::Face::AdoptEdgeLoop(Edge& edge)
{
	for (auto& e : EdgeLoop(edge))
	{
		auto it = e.face->FindEdge(e);
		m_edges.push_back(std::move(*it));
		e.face->m_edges.erase(it);
		e.face = this;
	}
}

EdgeMesh::Face* EdgeMesh::Face::DissolveEdge(Edge& edge, std::vector<Polygon>* newHoles)
{
	KERNEL_ASSERT(edge.twin);
	KERNEL_ASSERT(edge.face == this);

	Face* otherFace = edge.twin->face == this ? nullptr : edge.twin->face;

	AssertValid();
	
	if (otherFace)
	{
		otherFace->AssertValid();
		AdoptEdgeLoop(*edge.twin);
	}

	Edge& oldPrev = *edge.prev;
	Edge& newNext = *edge.twin->next;

	Edge& newPrev = *edge.twin->prev;
	Edge& oldNext = *edge.next;

	oldPrev.ConnectTo(newNext);
	newPrev.ConnectTo(oldNext);

	m_edges.erase(FindEdge(*edge.twin));
	m_edges.erase(FindEdge(edge));

	if (!otherFace)
	{
		KERNEL_ASSERT(newHoles);
		if (newHoles)
		{
			Face hole;
			hole.AdoptEdgeLoop(newPrev);

			if (hole.GetPolygon().IsCW()) // Got the wrong bit!
				swap(*this, hole);
			Polygon holePoly = hole.GetPolygon();
			holePoly.MakeCW();
			newHoles->push_back(holePoly);
		}
	}


	AssertValid();

	return otherFace;
}

void EdgeMesh::Face::AssertValid() const
{
#ifndef _DEBUG
	return;
#endif

	int n = 0;
	for (auto& e : GetEdges())
	{
		KERNEL_ASSERT(e.prev->next == &e);
		KERNEL_ASSERT(e.next->prev == &e);
		KERNEL_ASSERT(e.face == this);
		KERNEL_ASSERT(std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgePtr& edge) { return edge.get() == &e; }) != m_edges.end());
		KERNEL_ASSERT(!e.twin || e.twin->twin == &e);
		KERNEL_ASSERT(!e.twin || e.twin->next->vert == e.vert);
		++n;
	}
	
	KERNEL_ASSERT(n == m_edges.size());
}

bool EdgeMesh::Face::IsConcave() const
{
	for (auto& e : GetEdges())
		if (e.IsConcave())
			return true;
	return false;
}

bool EdgeMesh::Face::Contains(const Vec2& point) const
{
	if (!m_bbox.Contains(point))
		return false; 

	return Geometry::PointInPolygon(GetPointPairLoop(), point);
}

bool EdgeMesh::Face::Contains(const PolyLine& poly) const
{
	if (poly.empty())
		return false;

	for (auto& point : poly)
		if (!m_bbox.Contains(point))
			return false;

	return Geometry::PolygonContainsPolyline(GetPointPairLoop(), poly.GetPointPairLoop());
}

bool EdgeMesh::Face::DissolveToFit(const PolyLine& poly, std::vector<Face*>& deletedFaces, std::vector<Polygon>& newHoles)
{
	if (!Contains(poly[0]))
		return false;

	auto DissolveIntersectingEdge = [&](const Line2& line)
	{
		for (auto& edge : GetEdges())
			if (line.Intersect(edge.GetLine()))
			{
				if (Face* merged = DissolveEdge(edge, &newHoles))
					deletedFaces.push_back(merged);
				AssertValid();
				return true;
			}
		return false;
	};

	bool changed = false;
	do
	{
		for (auto& line : poly.GetLineLoop())
			if (changed = DissolveIntersectingEdge(line))
				break;
	} while (changed);

	AssertValid();
	return true;
}

void EdgeMesh::Face::Dump() const
{
	Debug::Trace << "Face " << std::hex << this << std::endl;

	for (auto& e : GetEdges())
		e.Dump();
}

void EdgeMesh::Face::Update()
{
	AssertValid();
	m_bbox = Geometry::GetBBox(GetPointLoop());
}

//-----------------------------------------------------------------------------

EdgeMesh::Edge::Edge() : face{}, prev{}, next{}, twin{}
{
}

EdgeMesh::Edge::Edge(const Vert* _vert, Face* _face, Edge* _prev, Edge* _next, Edge* _twin) :
	vert(_vert), face(_face), prev(_prev), next(_next), twin(_twin)
{
}

void EdgeMesh::Edge::Save(Kernel::Serial::SaveNode& node) const
{
	__super::Save(node);

	node.SaveObjectID(this);
	node.SaveObjectRef("vert", vert);
	node.SaveObjectRef("prev", prev);
	node.SaveObjectRef("next", next);
	node.SaveObjectRef("twin", twin);
}

void EdgeMesh::Edge::Load(const Kernel::Serial::LoadNode& node)
{
	__super::Load(node);

	node.LoadObjectID(this);
	node.LoadObjectRef("vert", vert);
	node.LoadObjectRef("prev", prev);
	node.LoadObjectRef("next", next);
	node.LoadObjectRef("twin", twin);
}

Vec2 EdgeMesh::Edge::GetVec() const
{
	return *next->vert - *vert;
}

double EdgeMesh::Edge::GetAngle() const
{
	return prev->GetVec().Normalised().GetAngle(GetVec().Normalised());
}

bool EdgeMesh::Edge::DoesVectorPointInside(const Jig::Vec2& vec) const
{
	KERNEL_ASSERT(vec.IsNormalised());
	auto prevVec = prev->GetVec().Normalised();
	return prevVec.GetAngle(vec) > prevVec.GetAngle(GetVec().Normalised());
}

bool EdgeMesh::Edge::IsRedundant() const
{
	if (!twin)
		return false;

	if (prev->GetVec().Normalised().GetAngle(twin->next->GetVec().Normalised()) < 0) 
		return false;

	if (twin->prev->GetVec().Normalised().GetAngle(next->GetVec().Normalised()) < 0) 
		return false;

	return true; // Both convex.
}

bool EdgeMesh::Edge::IsConnectedTo(const Edge& edge) const
{
	return &edge == this || &edge == next || &edge == prev;
}

Line2 EdgeMesh::Edge::GetLine() const
{
	return Line2::MakeFinite(*vert, *next->vert);
}

const EdgeMesh::Face* EdgeMesh::Edge::GetTwinFace() const
{
	return twin ? twin->face : nullptr;
}

// CCW
const EdgeMesh::Edge* EdgeMesh::Edge::GetPrevShared() const
{
	return twin ? twin->next : nullptr;
}

// CW
const EdgeMesh::Edge* EdgeMesh::Edge::GetNextShared() const
{
	return prev->twin;
}

const EdgeMesh::Edge* EdgeMesh::Edge::FindSharedEdge(const Face& otherFace) const
{
	if (face == &otherFace)
		return this;

	const Edge* edge = this;
	while ((edge = edge->GetNextShared()) && edge != this)
		if (edge->face == &otherFace)
			return edge;

	edge = this;
	while ((edge = edge->GetPrevShared()) && edge != this)
		if (edge->face == &otherFace)
			return edge;

	return nullptr;
}

// Returns outer edge that this connects to (shares next->vert). 
EdgeMesh::Edge* EdgeMesh::Edge::FindNextOuterEdge()
{
	Edge* edge = next;
	while (edge->twin)
	{
		edge = edge->twin->next;
		if (edge == next)
			return nullptr;
	}

	return edge;
}

// Returns outer edge that connects to this.
EdgeMesh::Edge* EdgeMesh::Edge::FindPrevOuterEdge()
{
	Edge* edge = prev;
	while (edge->twin)
	{
		edge = edge->twin->prev;
		if (edge == prev)
			return nullptr;
	}

	return edge;
}

// Returns outer edge that shares this->vert. 
EdgeMesh::Edge* EdgeMesh::Edge::FindSharedOuterEdge()
{
	return prev->FindNextOuterEdge();
}

void EdgeMesh::Edge::ConnectTo(Edge& edge)
{
	next = &edge;
	edge.prev = this;
}

void EdgeMesh::Edge::SetTwin(Edge& edge) 
{
	twin = &edge;
	edge.twin = this;
}

void EdgeMesh::Edge::Dump() const
{
	Debug::Trace << "  Edge:" << std::hex << this;
	Debug::Trace << " x:" << vert->x << " y:" << vert->y;
	Debug::Trace << " twin:" << std::hex << twin << std::endl;
}

void Jig::swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs)
{
	std::swap(lhs.m_edges, rhs.m_edges);

	for (auto& edge : lhs.GetEdges())
		edge.face = &lhs;

	for (auto& edge : rhs.GetEdges())
	edge.face = &rhs;
}