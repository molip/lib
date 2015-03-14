#include "EdgeMesh.h"

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

EdgeMesh::FacePtr EdgeMesh::Face::Split(Edge& e0, Edge& e1)
{
	FacePtr newFace = std::make_unique<Face>();

	// Clone edges.
	EdgePtr newStart = std::make_unique<Edge>(e0);
	EdgePtr newEnd = std::make_unique<Edge>(e1);

	// Connect transferred edges to new edge.
	newEnd->prev->next = newEnd.get();
	newStart->next->prev = newStart.get();

	// Connect new face's new edge.
	newEnd->next = newStart.get();
	newStart->prev = newEnd.get();

	// Connect old face's new edge.
	e0.next = &e1;
	e1.prev = &e0;

	// Twin up.
	e0.twin = newEnd.get();
	newEnd->twin = &e0;

	// Transfer edges to newFace.
	for (auto& edge : EdgeRange<Edge>(*newStart))
	{
		if (&edge != newStart.get() && &edge != newEnd.get())
		{
			auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const EdgePtr& e) { return e.get() == &edge; });
			assert(it != m_edges.end());
			newFace->m_edges.push_back(std::move(*it));
			m_edges.erase(it);
		}
		edge.face = newFace.get();
	}

	newFace->m_edges.push_back(std::move(newStart));
	newFace->m_edges.push_back(std::move(newEnd));

	assert(IsValid());
	assert(newFace->IsValid());

	return newFace;
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

