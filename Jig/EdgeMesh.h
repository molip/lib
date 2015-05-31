#pragma once

#include "Line2.h"
#include "Util.h"
#include "Vector.h"

#include <vector>
#include <set>
#include <memory>

namespace Jig
{
	class Polygon;

	class EdgeMesh
	{
	public:
		typedef Vec2 Vert;
		class Edge;
		class Face;

		EdgeMesh() {}
		EdgeMesh(EdgeMesh&& rhs);
		EdgeMesh(std::vector<EdgeMesh::Vert>&& verts);
		EdgeMesh(const EdgeMesh& rhs) = delete;

		void operator=(EdgeMesh&& rhs);

		typedef const Vert* VertPtr;
		typedef std::unique_ptr<Edge> EdgePtr;
		typedef std::unique_ptr<Face> FacePtr;

		void AddFace(FacePtr face);

		void DeleteFace(Face& face);
		Face& SplitFace(Face& face, Edge& e0, Edge& e1);
		void DissolveEdge(Edge& edge);
		void DissolveRedundantEdges();

		const Face* HitTest(const Vec2& point) const;
		bool Contains(const Polygon& poly) const;

		void Clear() { m_faces.clear(); }
		const std::vector<FacePtr>& GetFaces() const { return m_faces; }
		const std::vector<Vert>& GetVerts() const { return m_verts; }

		template <typename T>
		class EdgeIter
		{
		public:
			EdgeIter(T& start, bool started = false) : m_started(started), m_current(&start) {}
			bool operator !=(const EdgeIter<T>& rhs) const { return m_current != rhs.m_current || m_started != rhs.m_started; }
			T& operator* () const { return *m_current; }
			void operator++ () { m_current = m_current->next; m_started = true; }
		private:
			T* m_current;
			bool m_started;
		};

		class LineIter
		{
		public:
			LineIter(EdgeIter<const Edge> iter) : m_iter(iter) {}
			bool operator !=(const LineIter& rhs) const { return m_iter != rhs.m_iter; }
			Line2 operator* () const { return Line2::MakeFinite(*(*m_iter).vert, *(*m_iter).next->vert); }
			void operator++ () { ++m_iter; }
		private:
			EdgeIter<const Edge> m_iter;
		};
		
		class EdgeLoop : public Util::Iterable<EdgeIter<Edge>>
		{
		public:
			EdgeLoop(Edge& edge) : Util::Iterable<EdgeIter<Edge>>(EdgeIter<Edge>(edge), EdgeIter<Edge>(edge, true)) {}
		};
		class ConstEdgeLoop : public Util::Iterable<EdgeIter<const Edge>>
		{
		public:
			ConstEdgeLoop(const Edge& edge) : Util::Iterable<EdgeIter<const Edge>>(EdgeIter<const Edge>(edge), EdgeIter<const Edge>(edge, true)) {}
			ConstEdgeLoop(const Edge& start, const Edge& end) : Util::Iterable<EdgeIter<const Edge>>(EdgeIter<const Edge>(start), EdgeIter<const Edge>(end, true)) {}
		};

		class LineLoop : public Util::Iterable<LineIter>
		{
		public:
			LineLoop(const Edge& edge) : Util::Iterable<LineIter>(LineIter(EdgeIter<const Edge>(edge)), LineIter(EdgeIter<const Edge>(edge, true))) {}
		};

		class Edge
		{
		public:
			Edge();
			Edge(const Edge& rhs) = delete;
			Edge(Face* _face, VertPtr _vert, Edge* _prev = nullptr, Edge* next = nullptr, Edge* twin = nullptr);

			Vec2 GetVec() const;
			double GetAngle() const;
			bool IsConcave() const { return GetAngle() < 0; }
			bool IsRedundant() const;
			bool IsConnectedTo(const Edge& edge) const;
			Line2 GetLine() const;
			const Face* GetTwinFace() const;

			void ConnectTo(Edge& edge);
			void BridgeTo(Edge& edge);
			void Dump() const;

			Face* face;
			VertPtr vert;
			Edge *prev, *next, *twin;
		};

		class Face
		{
			friend class EdgeMesh;
			friend class Edge;
			friend void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
		public:
			Face() {}
			Face(const Face& rhs) = delete;

			Edge& AddAndConnectEdge(VertPtr vert);

			Edge& GetEdge() { return **m_edges.begin(); }
			const Edge& GetEdge() const { return **m_edges.begin(); }
			EdgeLoop GetEdges() { return EdgeLoop(GetEdge()); }
			ConstEdgeLoop GetEdges() const { return ConstEdgeLoop(GetEdge()); }
			ConstEdgeLoop GetOtherEdges(const Edge& edge) const { return ConstEdgeLoop(*edge.next, edge); }
			LineLoop GetLineLoop() const { return LineLoop(GetEdge()); }

			int GetEdgeCount() const { return (int)m_edges.size(); }
			Polygon GetPolygon() const;
			bool IsValid() const;
			bool IsConcave() const;
			bool Contains(const Vec2& point) const;
			bool Contains(const Polygon& poly) const;

			bool DissolveToFit(const Polygon& poly, std::vector<Face*>& deletedFaces, std::vector<Polygon>& newHoles);

			void Bridge(Edge& e0, Edge& e1);

			void Dump() const;

		private:
			Edge& AddEdge(VertPtr vert);
			FacePtr Split(Edge& e0, Edge& e1);
			EdgeMesh::Face* DissolveEdge(Edge& edge, std::vector<Polygon>* newHoles);
			std::vector<EdgePtr>::iterator FindEdge(Edge& edge);
			void AdoptEdgeLoop(Edge& edge);

			std::vector<EdgePtr> m_edges; // Unordered.
		};

	private:
		void Convexify(Face& face);
		bool DissolveRedundantEdges(Face& face);

		double GetAngle(const Edge& edge) const;
		Vec2 GetVec(const Edge& edge) const;

		std::vector<FacePtr> m_faces;
		std::vector<Vert> m_verts;
	};
void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
}

