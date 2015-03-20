#include "EdgeMesh.h"

#include "Debug.h"
#include "Geometry.h"
#include "ShapeSplitter.h"
#include "Polygon.h"

#include <cassert>

using namespace Jig;

EdgeMesh::EdgeMesh(EdgeMesh&& rhs) : m_faces(std::move(rhs.m_faces))
{
}

void EdgeMesh::Init(const Polygon& poly)
{
	m_faces.clear();
	m_faces.push_back(std::make_unique<Face>(poly));

	if (poly.size() >= 4)
		ShapeSplitter(*this).Convexify(*m_faces.back());
}

EdgeMesh::Face& EdgeMesh::SplitFace(Face& face, Edge& e0, Edge& e1)
{
	m_faces.push_back(face.Split(e0, e1));
	return *m_faces.back();
}

void EdgeMesh::DeleteFace(Face& face)
{
	auto it = std::find_if(m_faces.begin(), m_faces.end(), [&](const FacePtr& f) { return f.get() == &face; });
	assert(it != m_faces.end());
	m_faces.erase(it);
}

void EdgeMesh::DissolveEdge(Edge& edge)
{
	DeleteFace(edge.face->DissolveEdge(edge));
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

// Assumes this contains poly.
bool EdgeMesh::AddHole(const Polygon& poly)
{
	assert(poly.IsCW());

	Polygon reversed;
	reversed.insert(reversed.end(), poly.crbegin(), poly.crend());

	std::vector<Face*> deletedFaces;

	for (auto& face : m_faces)
		if (face->DissolveToFit(poly, deletedFaces))
		{
			assert(face->Contains(poly));

			ShapeSplitter(*this).AddHole(*face, Face(reversed));

			for (auto& face : deletedFaces)
				DeleteFace(*face);
			return true;
		}
	return false;
}

//-----------------------------------------------------------------------------

EdgeMesh::Face::Face(const Polygon& poly)
{
	//assert(poly.size() >= 3 && poly.IsCW());

	Edge* prev = nullptr;
	Edge* last = nullptr;
	for (auto& vert : poly)
	{
		Edge& e = AddEdge(std::make_shared<Vert>(vert));
		if (!prev)
			prev = last = &e;
		else
		{
			prev->ConnectTo(e);
			e.ConnectTo(*last);
			prev = &e;
		}
	}
}

Polygon EdgeMesh::Face::GetPolygon() const
{
	Polygon poly;
	for (auto& edge : GetEdges())
		poly.push_back(*edge.vert);
	return poly;
}

EdgeMesh::Edge& EdgeMesh::Face::AddEdge(VertPtr vert)
{
	m_edges.push_back(std::make_unique<Edge>(this, vert));
	return *m_edges.back();
}

std::vector<EdgeMesh::EdgePtr>::iterator EdgeMesh::Face::FindEdge(Edge& edge)
{
	auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgePtr& e) { return e.get() == &edge; });
	assert(it != m_edges.end());
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

EdgeMesh::FacePtr EdgeMesh::Face::Split(Edge& e0, Edge& e1)
{
	assert(IsValid());
	assert(e0.face == this && e1.face == this);

	FacePtr newFace = std::make_unique<Face>();

	e0.BridgeTo(e1); // e0 loop now separate. 

	newFace->AdoptEdgeLoop(e0);

	assert(IsValid());
	assert(newFace->IsValid());

	return newFace;
}

void EdgeMesh::Face::Bridge(Edge& e0, Edge& e1)
{
	assert(IsValid());
	assert(e0.face == this && e1.face != this);

	AdoptEdgeLoop(e1);
	
	e0.BridgeTo(e1);

	assert(IsValid());
}

EdgeMesh::Face& EdgeMesh::Face::DissolveEdge(Edge& edge)
{
	assert(edge.twin);
	assert(edge.face == this);

	Face* otherFace = edge.twin->face;

	assert(IsValid());
	assert(otherFace->IsValid());

	AdoptEdgeLoop(*edge.twin);
		
	Edge& oldEnd = *edge.prev;
	Edge& newStart = *edge.twin->next;

	Edge& newEnd = *edge.twin->prev;
	Edge& oldStart = *edge.next;

	oldEnd.ConnectTo(newStart);
	newEnd.ConnectTo(oldStart);

	m_edges.erase(FindEdge(*edge.twin));
	m_edges.erase(FindEdge(edge));

	assert(IsValid());

	return *otherFace;
}

bool EdgeMesh::Face::IsValid() const
{
	if (m_edges.size() < 3)
		return false;

	int n = 0;
	for (auto& e : GetEdges())
	{
		if (e.prev->next != &e)
			return false;

		if (e.next->prev != &e)
			return false;

		if (e.face != this)
			return false;

		if (std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgePtr& edge) { return edge.get() == &e; }) == m_edges.end())
			return false;

		if (e.twin && e.twin->twin != &e)
			return false;

		++n;
	}
	
	if (n != m_edges.size())
		return false;

	return true;
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
	return Geometry::PointInPolygon(GetLineLoop(), point);
}

bool EdgeMesh::Face::Contains(const Polygon& poly) const
{
	return Geometry::PolygonContainsPolygon(GetLineLoop(), poly.GetLineLoop());
}

bool EdgeMesh::Face::DissolveToFit(const Polygon& poly, std::vector<Face*>& deletedFaces)
{
	if (!Contains(poly[0]))
		return false;

	auto DissolveIntersectingEdge = [&](const Line2& line)
	{
		for (auto& edge : GetEdges())
			if (line.Intersect(edge.GetLine()))
			{
				deletedFaces.push_back(&DissolveEdge(edge));
				assert(IsValid());
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

	return true;
}

void EdgeMesh::Face::Dump() const
{
	Debug::Trace << "Face " << std::hex << this << std::endl;

	for (auto& e : GetEdges())
		e.Dump();
}

//-----------------------------------------------------------------------------

EdgeMesh::Edge::Edge() : face{}, prev{}, next{}, twin{}
{
}

EdgeMesh::Edge::Edge(Face* _face, VertPtr _vert, Edge* _prev, Edge* _next, Edge* _twin) :
	face(_face), vert(_vert), prev(_prev), next(_next), twin(_twin)
{
}

Vec2 EdgeMesh::Edge::GetVec() const
{
	return *next->vert - *vert;
}

double EdgeMesh::Edge::GetAngle() const
{
	return prev->GetVec().Normalised().GetAngle(GetVec().Normalised());
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

void EdgeMesh::Edge::ConnectTo(Edge& edge)
{
	next = &edge;
	edge.prev = this;
}

// Creates 2 new edges, connecting this->prev to edge and edge.prev to this. 
void EdgeMesh::Edge::BridgeTo(Edge& edge)
{
	// Add edges.
	Edge& new0 = edge.face->AddEdge(vert);
	Edge& new1 = edge.face->AddEdge(edge.vert);

	// Connect new edges to prev.
	prev->ConnectTo(new0);
	edge.prev->ConnectTo(new1);

	// Connect new edges to next.
	new0.ConnectTo(edge);
	new1.ConnectTo(*this);

	// Twin up.
	new0.twin = &new1;
	new1.twin = &new0;
}

void EdgeMesh::Edge::Dump() const
{
	Debug::Trace << "  Edge:" << std::hex << this;
	Debug::Trace << " x:" << vert->x << " y:" << vert->y;
	Debug::Trace << " twin:" << std::hex << twin << std::endl;
}
