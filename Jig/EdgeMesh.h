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
		EdgeMesh() {}
		EdgeMesh(EdgeMesh&& rhs);
		EdgeMesh(const EdgeMesh& rhs) = delete;

		void Init(const Polygon& poly, bool optimise);

		typedef Vec2 Vert;
		class Edge;
		class Face;

		void DeleteFace(Face& face);
		Face& SplitFace(Face& face, Edge& e0, Edge& e1);
		void DissolveEdge(Edge& edge);
		void DissolveRedundantEdges();

		bool Contains(const Polygon& poly) const;
		bool DissolveToFit(const Polygon& poly);

		typedef std::shared_ptr<Vert> VertPtr;
		typedef std::unique_ptr<Edge> EdgePtr;
		typedef std::unique_ptr<Face> FacePtr;

		void Clear() { m_faces.clear(); }
		const std::vector<FacePtr>& GetFaces() const { return m_faces; }

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
			Edge(Face* _face, VertPtr _vert, Edge* _prev = nullptr, Edge* next = nullptr, Edge* twin = nullptr);

			Vec2 GetVec() const;
			double GetAngle() const;
			bool IsConcave() const { return GetAngle() < 0; }
			bool IsRedundant() const;
			Line2 GetLine() const;

			Face* face;
			VertPtr vert;
			Edge *prev, *next, *twin;
		};

		class Face
		{
			friend class EdgeMesh;
		public:
			Face() {}
			Face(const Face& rhs) = delete;
			Face(const Face&& rhs);
			Face(const Polygon& poly);

			Edge& GetEdge() { return **m_edges.begin(); }
			const Edge& GetEdge() const { return **m_edges.begin(); }
			EdgeLoop GetEdges() { return EdgeLoop(GetEdge()); }
			ConstEdgeLoop GetEdges() const { return ConstEdgeLoop(GetEdge()); }
			LineLoop GetLineLoop() const { return LineLoop(GetEdge()); }

			int GetEdgeCount() const { return (int)m_edges.size(); }
			Polygon GetPolygon() const;
			bool IsValid() const;
			bool IsConcave() const;
			bool Contains(const Vec2& point) const;

			bool DissolveToFit(const Polygon& poly, std::vector<Face*>& deletedFaces);

			Edge& AddEdgeAfter(Edge& prev, VertPtr vert, Edge* twin = nullptr);
			
		private:
			FacePtr Split(Edge& e0, Edge& e1);
			EdgeMesh::Face& DissolveEdge(Edge& edge);
			void Connect(Edge& first, Edge& second);
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
	};
}