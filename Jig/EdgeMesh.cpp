#include "EdgeMesh.h"

#include "Geometry.h"
#include "ShapeSplitter.h"
#include "Polygon.h"

#include <cassert>

using namespace Jig;

EdgeMesh::EdgeMesh(EdgeMesh&& rhs) : m_faces(std::move(rhs.m_faces))
{
}

void EdgeMesh::Init(const Polygon& poly, bool optimise)
{
	m_faces.clear();
	m_faces.push_back(std::make_unique<Face>(poly));

	if (poly.size() >= 4)
		ShapeSplitter(*this).Convexify(*m_faces.back());

	if (optimise)
		DissolveRedundantEdges();
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
bool EdgeMesh::DissolveToFit(const Polygon& poly)
{
	std::vector<Face*> deletedFaces;

	for (auto& face : m_faces)
		if (face->DissolveToFit(poly, deletedFaces))
		{
			for (auto& face : deletedFaces)
				DeleteFace(*face);
			return true;
		}
	return false;
}

//-----------------------------------------------------------------------------

EdgeMesh::Face::Face(const Polygon& poly)
{
	assert(poly.size() >= 3 && poly.IsCW());

	Edge* prev = nullptr;
	for (auto& vert : poly)
	{
		if (m_edges.empty())
		{
			m_edges.push_back(std::make_unique<Edge>(this, std::make_shared<Vert>(vert)));
			Edge& e = *m_edges.back();
			prev = e.prev = e.next = &e;

		}
		else
			prev = &AddEdgeAfter(*prev, std::make_shared<Vert>(vert));
	}
}

Polygon EdgeMesh::Face::GetPolygon() const
{
	Polygon poly;
	for (auto& edge : GetEdges())
		poly.push_back(*edge.vert);
	return poly;
}

EdgeMesh::Edge& EdgeMesh::Face::AddEdgeAfter(Edge& prev, VertPtr vert, Edge* twin)
{
	m_edges.push_back(std::make_unique<Edge>(this, vert, &prev, prev.next, twin));
	Edge& e = *m_edges.back();
	prev.next = e.next->prev = &e;
	return e;
}

void EdgeMesh::Face::Connect(Edge& first, Edge& second)
{
	first.next = &second;
	second.prev = &first;
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

	FacePtr newFace = std::make_unique<Face>();

	// Clone edges.
	m_edges.push_back(std::make_unique<Edge>(e0));
	Edge* new0 = m_edges.back().get();
	m_edges.push_back(std::make_unique<Edge>(e1));
	Edge* new1 = m_edges.back().get();

	// Connect new edges to prev.
	new0->prev->next = new0;
	new1->prev->next = new1;

	// Connect new edges to next.
	Connect(*new0, e1);
	Connect(*new1, e0);

	// Twin up.
	new0->twin = new1;
	new1->twin = new0;

	newFace->AdoptEdgeLoop(*new1);

	assert(IsValid());
	assert(newFace->IsValid());

	return newFace;
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

	Connect(oldEnd, newStart);
	Connect(newEnd, oldStart);

	m_edges.erase(FindEdge(*edge.twin));
	m_edges.erase(FindEdge(edge));

	assert(IsValid());

	return *otherFace;
}

bool EdgeMesh::Face::IsValid() const
{
	if (m_edges.size() < 3)
		return false;

	if (!GetPolygon().IsCW())
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

		if (e.twin && e.twin->face == this)
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

Line2 EdgeMesh::Edge::GetLine() const
{
	return Line2::MakeFinite(*vert, *next->vert);
}

