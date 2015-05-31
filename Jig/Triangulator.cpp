#include "Triangulator.h"

#include "Polygon.h"

#include <poly2tri.h>

#include <map>

using namespace Jig;

Triangulator::Triangulator(const Polygon& poly) : m_poly(poly)
{
}

Triangulator::~Triangulator()
{
}

void Triangulator::AddHole(const Polygon& poly)
{
	m_holes.push_back(&poly);
}

EdgeMesh Triangulator::Go()
{
	std::vector<p2t::Point> points;
	typedef std::vector<p2t::Point*> Polyline;
	std::vector<Polyline> polylines;

	size_t pointCount = m_poly.size();
	for (auto& h: m_holes)
		pointCount += h->size();

	points.reserve(pointCount);

	auto Convert = [&](const Polygon& poly)
	{
		polylines.emplace_back();
		Polyline& polyline = polylines.back();
		polyline.reserve(m_poly.size());
		for (auto& p : poly)
		{
			points.emplace_back(p.x, p.y);
			polyline.push_back(&points.back());
		}
		return polyline;
	};
	
	p2t::CDT cdt(Convert(m_poly));
	for (auto& h : m_holes)
		cdt.AddHole(Convert(*h));

	cdt.Triangulate();

	EdgeMesh mesh;

	std::map<p2t::Point*, EdgeMesh::VertPtr> pointToVert;
	std::map<std::pair<EdgeMesh::Vert*, EdgeMesh::Vert*>, EdgeMesh::Edge*> vertsToEdge;

	for (auto& p : points)
		pointToVert[&p] = std::make_shared<EdgeMesh::Vert>(p.x, p.y);

	for (auto* tri : cdt.GetTriangles())
	{
		EdgeMesh::FacePtr face = std::make_unique<EdgeMesh::Face>();

		bool hasTwin[3] = {};
		for (int i = 0; i < 3; ++i)
		{
			auto* point = tri->GetPoint(i);
			face->AddAndConnectEdge(pointToVert[point]);
			hasTwin[i] = tri->NeighborCCW(*point) != nullptr;
		}

		// Connect twin edges.
		int i = 0;
		for (auto& edge : face->GetEdges())
		{
			if (hasTwin[i++]) // Don't bother checking/saving otherwise.
			{
				auto* vert0 = edge.vert.get();
				auto* vert1 = edge.next->vert.get();

				auto it = vertsToEdge.find(std::make_pair(vert1, vert0));
				if (it == vertsToEdge.end()) // Haven't found the twin yet. 
					vertsToEdge[std::make_pair(vert0, vert1)] = &edge;
				else
				{
					edge.twin = it->second;
					edge.twin->twin = &edge;
					vertsToEdge.erase(it); // Completed this edge pair. 
				}
			}
		}

		mesh.AddFace(std::move(face));
	}
	return mesh;
}

