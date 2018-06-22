#pragma once

#include "Line2.h"
#include "QuadTree.h"
#include "Vector.h"

#include "libKernel/Util.h"

#include <vector>
#include <set>
#include <memory>

namespace Jig
{
	class Polygon;
	class PolyLine;

	class EdgeMesh
	{
	public:
		class Vert;
		class Edge;
		class Face;

		typedef std::unique_ptr<Vert> VertPtr;
		typedef std::unique_ptr<Edge> EdgePtr;
		typedef std::unique_ptr<Face> FacePtr;

		class Vert : public Vec2
		{
		public:
			using Vec2::Vec2;
			using VisibleVec = std::vector<const Vert*>;

			VisibleVec visible;
		};

		EdgeMesh() {}
		EdgeMesh(EdgeMesh&& rhs);
		EdgeMesh(std::vector<VertPtr>&& verts);
		EdgeMesh(const EdgeMesh& rhs) = delete;

		void operator=(EdgeMesh&& rhs);

		void SetEnableVisiblePoints(bool val) { m_enableVisiblePoints = val; }

		static std::unique_ptr<Vert> MakeVert(const Vec2& point) { return std::make_unique<Vert>(point); }
		static FacePtr MakeTwinFace(Edge& start, Edge& end);

		Face& AddFace(FacePtr face);
		Vert& AddVert(const Vec2& point);
		Edge& InsertVert(const Vec2& point, Edge& edge);

		void DeleteFace(Face& face);
		Face& SplitFace(Face& face, Edge& e0, Edge& e1);
		void DissolveEdge(Edge& edge);
		void DissolveRedundantEdges();

		const Face* HitTest(const Vec2& point) const;
		const Vert* FindNearestVert(const Vec2& point, double tolerance = -1) const;
		bool Contains(const Polygon& poly) const;

		void Clear() { m_faces.clear(); }
		const std::vector<FacePtr>& GetFaces() const { return m_faces; }
		const std::vector<VertPtr>& GetVerts() const { return m_verts; }
		Edge* FindOuterEdge();
		const Edge* FindOuterEdge() const { return const_cast<EdgeMesh*>(this)->FindOuterEdge(); }
		Edge* FindOuterEdgeWithVert(const Vert& vert);
		const Edge* FindOuterEdgeWithVert(const Vert& vert) const { return const_cast<EdgeMesh*>(this)->FindOuterEdgeWithVert(vert); }

		void Update();

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

		template <typename T>
		class OuterEdgeIter
		{
		public:
			OuterEdgeIter(T& start, bool started = false) : m_started(started), m_current(&start) { KERNEL_ASSERT(!start.twin); }
			bool operator !=(const OuterEdgeIter<T>& rhs) const { return m_current != rhs.m_current || m_started != rhs.m_started; }
			T& operator* () const { return *m_current; }
			void operator++ () 
			{
				auto* old = m_current;
				m_started = true;
				m_current = m_current->next->FindSharedOuterEdge();
				KERNEL_ASSERT(m_current && !m_current->twin);
			}
		private:
			T * m_current;
			bool m_started;
		};

		template <typename IterT>
		class MetaIter
		{
		public:
			MetaIter(const Edge& iter, bool started = false) : m_iter(iter, started) {}
			bool operator !=(const MetaIter& rhs) const { return m_iter != rhs.m_iter; }
			void operator++ () { ++m_iter; }
		protected:
			IterT m_iter;
		};

		template <typename IterT>
		class LineIter : public MetaIter<IterT>
		{
		public:
			using MetaIter::MetaIter;
			Line2 operator* () const { return Line2::MakeFinite(*(*m_iter).vert, *(*m_iter).next->vert); }
		};

		template <typename IterT>
		class PointPairIter : public MetaIter<IterT>
		{
		public:
			using MetaIter::MetaIter;
			std::pair<Vec2, Vec2> operator* () const { return std::pair<Vec2, Vec2>(*(*m_iter).vert, *(*m_iter).next->vert); } // Don't use make_pair!
		};

		template <typename IterT>
		class PointIter : public MetaIter<IterT>
		{
		public:
			using MetaIter::MetaIter;
			Vec2 operator* () const { return *(*m_iter).vert; }
		};

		template <typename IterT>
		class Loop : public Kernel::Iterable<IterT>
		{
		public:
			Loop(Edge& edge) : Iterable(IterT(edge), IterT(edge, true)) {}
			Loop(const Edge& edge) : Iterable(IterT(edge), IterT(edge, true)) {}

			Loop(Edge& start, Edge& end) : Iterable(IterT(start), IterT(end, true)) {}
			Loop(const Edge& start, const Edge& end) : Iterable(IterT(start), IterT(end, true)) {}
		};

		typedef Loop<EdgeIter<Edge>> EdgeLoop;
		typedef Loop<EdgeIter<const Edge>> ConstEdgeLoop;
		typedef Loop<OuterEdgeIter<Edge>> OuterEdgeLoop;
		typedef Loop<OuterEdgeIter<const Edge>> ConstOuterEdgeLoop;

		typedef Loop<LineIter<EdgeIter<const Edge>>> LineLoop;
		typedef Loop<PointIter<EdgeIter<const Edge>>> PointLoop;
		typedef Loop<PointPairIter<EdgeIter<const Edge>>> PointPairLoop;
		typedef Loop<PointIter<OuterEdgeIter<const Edge>>> OuterPointLoop;
		typedef Loop<PointPairIter<OuterEdgeIter<const Edge>>> OuterPointPairLoop;

		class Edge
		{
		public:
			Edge();
			Edge(const Edge& rhs) = delete;
			Edge(Face* _face, const Vert* _vert, Edge* _prev = nullptr, Edge* next = nullptr, Edge* twin = nullptr);

			Vec2 GetVec() const;
			double GetAngle() const;
			bool IsConcave() const { return GetAngle() < 0; }
			bool DoesVectorPointInside(const Jig::Vec2& vec) const;
			bool IsRedundant() const;
			bool IsConnectedTo(const Edge& edge) const;
			Line2 GetLine() const;
			const Face* GetTwinFace() const;
			const Edge* FindSharedEdge(const Face& face) const;
			Edge* FindSharedOuterEdge();
			const Edge* FindSharedOuterEdge() const { return const_cast<Edge*>(this)->FindSharedOuterEdge(); }
			void ConnectTo(Edge& edge);
			Edge& BridgeTo(Edge& edge);
			void SetTwin(Edge& edge);
			void Dump() const;

			Face* face;
			const Vert* vert;
			Edge *prev, *next, *twin;
		};

		class Face
		{
			friend class EdgeMesh;
			friend class Edge;
			friend void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
		public:
			Face() {}
			Face(Edge& edgeLoopToAdopt);
			Face(const Face& rhs) = delete;

			Edge& AddAndConnectEdge(const Vert* vert, Edge* after = nullptr);

			Edge& GetEdge() { return **m_edges.begin(); }
			const Edge& GetEdge() const { return **m_edges.begin(); }
			EdgeLoop GetEdges() { return EdgeLoop(GetEdge()); }
			ConstEdgeLoop GetEdges() const { return ConstEdgeLoop(GetEdge()); }
			ConstEdgeLoop GetOtherEdges(const Edge& edge) const { return ConstEdgeLoop(*edge.next, edge); }
			LineLoop GetLineLoop() const { return LineLoop(GetEdge()); }
			PointPairLoop GetPointPairLoop() const { return PointPairLoop(GetEdge()); }
			PointLoop GetPointLoop() const{ return PointLoop(GetEdge()); }
			const Rect& GetBBox() const { return m_bbox; }
			Edge* FindOuterEdge();
			const Edge* FindOuterEdge() const { return const_cast<Face*>(this)->FindOuterEdge(); }
			Edge* FindEdgeWithVert(const Vert& vert);
			const Edge* FindEdgeWithVert(const Vert& vert) const { return const_cast<Face*>(this)->FindEdgeWithVert(vert); }

			int GetEdgeCount() const { return (int)m_edges.size(); }
			Polygon GetPolygon() const;
			void AssertValid() const;
			bool IsConcave() const;
			bool Contains(const Vec2& point) const;
			bool Contains(const PolyLine& poly) const;

			bool DissolveToFit(const PolyLine& poly, std::vector<Face*>& deletedFaces, std::vector<Polygon>& newHoles);

			void Bridge(Edge& e0, Edge& e1);
			
			void Update();

			void Dump() const;

		private:
			Edge& AddEdge(const Vert* vert);
			FacePtr Split(Edge& e0, Edge& e1);
			EdgeMesh::Face* DissolveEdge(Edge& edge, std::vector<Polygon>* newHoles);
			std::vector<EdgePtr>::iterator FindEdge(Edge& edge);
			void AdoptEdgeLoop(Edge& edge);

			std::vector<EdgePtr> m_edges; // Unordered.
			Rect m_bbox;
		};

	private:
		bool DissolveRedundantEdges(Face& face);

		std::vector<FacePtr> m_faces;
		std::vector<VertPtr> m_verts;
		QuadTree<Face> m_quadTree;
		Rect m_bbox;
		bool m_enableVisiblePoints = true;
	};
void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
}

