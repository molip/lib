#pragma once

#include "Line2.h"
#include "QuadTree.h"
#include "Vector.h"

#include "libKernel/Serial.h"
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
		class Data;

		typedef std::unique_ptr<Vert> VertPtr;
		typedef std::unique_ptr<Edge> EdgePtr;
		typedef std::unique_ptr<Face> FacePtr;
		typedef std::unique_ptr<Data> DataPtr;

		class Data : public Kernel::Dynamic
		{
		};

		class DataOwner
		{
		public:
			DataOwner() = default;
			DataOwner(const DataOwner&) = delete;
			DataOwner& operator=(const DataOwner&) = delete;

			void Save(Kernel::Serial::SaveNode& node) const { if (m_data) node.SaveObject("data", m_data); }
			void Load(const Kernel::Serial::LoadNode& node) { node.LoadObject("data", m_data); }

			void SetData(DataPtr data) { m_data = std::move(data); }
			const Data* GetData() const { return m_data.get(); }

		private:
			DataPtr m_data;
		};

		class Vert : public Vec2, public DataOwner
		{
		public:
			using Vec2::Vec2;
			void Save(Kernel::Serial::SaveNode& node) const { __super::Save(node); node.SaveType("pos", *(Vec2*)this); node.SaveObjectID(this); }
			void Load(const Kernel::Serial::LoadNode& node) { __super::Load(node); node.LoadType("pos", *(Vec2*)this); node.LoadObjectID(this); }
		};

		EdgeMesh() {}
		EdgeMesh(EdgeMesh&& rhs);
		EdgeMesh(std::vector<VertPtr>&& verts);
		EdgeMesh(const EdgeMesh& rhs) = delete;

		void Save(Kernel::Serial::SaveNode& node) const;
		void Load(const Kernel::Serial::LoadNode& node);

		void operator=(EdgeMesh&& rhs);

		static std::unique_ptr<Vert> MakeVert(const Vec2& point) { return std::make_unique<Vert>(point); }

		Vert& AddVert(const Vec2& point);
		Face& PushFace(EdgeMesh::FacePtr face);
		FacePtr PopFace();
		Vert& PushVert(VertPtr vert);
		VertPtr PopVert();
		std::pair<VertPtr, size_t> RemoveVert(Vert& vert);
		void InsertVert(VertPtr vert, size_t index);

		void DeleteFace(Face& face);
		void DissolveEdge(Edge& edge);
		void DissolveRedundantEdges();

		const Face* HitTest(const Vec2& point) const;
		const Vert* FindNearestVert(const Vec2& point, double tolerance = -1) const;
		bool Contains(const Polygon& poly) const;

		void Clear();
		const std::vector<FacePtr>& GetFaces() const { return m_faces; }
		const std::vector<VertPtr>& GetVerts() const { return m_verts; }
		Edge* FindOuterEdge();
		const Edge* FindOuterEdge() const { return const_cast<EdgeMesh*>(this)->FindOuterEdge(); }
		Edge* FindOuterEdgeWithVert(const Vert& vert);
		const Edge* FindOuterEdgeWithVert(const Vert& vert) const { return const_cast<EdgeMesh*>(this)->FindOuterEdgeWithVert(vert); }
		Edge* FindEdgeWithVert(const Vert& vert);
		const Edge* FindEdgeWithVert() const { return const_cast<EdgeMesh*>(this)->FindEdgeWithVert(); }

		void Update();

		void Dump() const;

		template <typename T>
		class EdgeIter
		{
		public:
			using Type = T;
			EdgeIter(T& start, bool started = false) : m_started(started), m_current(&start) {}
			bool operator !=(const EdgeIter<T>& rhs) const { return m_current != rhs.m_current || m_started != rhs.m_started; }
			T& operator* () const { return *m_current; }
			void operator++ () { m_current = m_current->next; m_started = true; }
			static T& GetPrev(T& edge) { return *edge.prev; }
			static T& GetNext(T& edge) { return *edge.next; }
		private:
			T* m_current;
			bool m_started;
		};

		template <typename T>
		class OuterEdgeIter
		{
		public:
			using Type = T;
			OuterEdgeIter(T& start, bool started = false) : m_started(started), m_current(&start) { KERNEL_ASSERT(!start.twin); }
			bool operator !=(const OuterEdgeIter<T>& rhs) const { return m_current != rhs.m_current || m_started != rhs.m_started; }
			T& operator* () const { return *m_current; }
			void operator++ () 
			{
				auto* old = m_current;
				m_started = true;
				m_current = &GetNext(*m_current);
				KERNEL_ASSERT(m_current && !m_current->twin);
			}
			static T& GetPrev(T& edge) { return *edge.FindPrevOuterEdge(); }
			static T& GetNext(T& edge) { return *edge.FindNextOuterEdge(); }
		private:
			T * m_current;
			bool m_started;
		};

		template <typename T>
		class SharedEdgeIter
		{
		public:
			using Type = T;
			SharedEdgeIter(T& start, bool started = false) : m_started(started), m_current(&start) {}
			bool operator !=(const SharedEdgeIter<T>& rhs) const { return m_current && (m_current != rhs.m_current || m_started != rhs.m_started); }
			T& operator* () const { return *m_current; }
			void operator++ () 
			{
				auto* old = m_current;
				m_started = true;
				m_current = &GetNext(*m_current);
			}
			static T& GetPrev(T& edge) { return *edge.GetPrevShared(); }
			static T& GetNext(T& edge) { return *edge.GetNextShared(); }
		private:
			T* m_current;
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

		template <typename IterT>
		class EdgeLoopT : public Loop<IterT>
		{
		public:
			using Loop::Loop;
			static typename IterT::Type& GetPrev(typename IterT::Type& edge) { return IterT::GetPrev(edge); }
			static typename IterT::Type& GetNext(typename IterT::Type& edge) { return IterT::GetNext(edge); }
		};

		typedef EdgeLoopT<EdgeIter<Edge>> EdgeLoop;
		typedef EdgeLoopT<EdgeIter<const Edge>> ConstEdgeLoop;
		
		typedef EdgeLoopT<OuterEdgeIter<Edge>> OuterEdgeLoop;
		typedef EdgeLoopT<OuterEdgeIter<const Edge>> ConstOuterEdgeLoop;

		typedef EdgeLoopT<SharedEdgeIter<Edge>> SharedEdges;
		typedef EdgeLoopT<SharedEdgeIter<const Edge>> ConstSharedEdges;

		typedef Loop<LineIter<EdgeIter<const Edge>>> LineLoop;
		typedef Loop<PointIter<EdgeIter<const Edge>>> PointLoop;
		typedef Loop<PointPairIter<EdgeIter<const Edge>>> PointPairLoop;
		typedef Loop<PointIter<OuterEdgeIter<const Edge>>> OuterPointLoop;
		typedef Loop<PointPairIter<OuterEdgeIter<const Edge>>> OuterPointPairLoop;

		// Caution - prev/next edges may not be outer! Use GetPrev/GetNext instead. 
		OuterEdgeLoop GetOuterEdges() { return OuterEdgeLoop(*FindOuterEdge()); }
		ConstOuterEdgeLoop GetOuterEdges() const { return ConstOuterEdgeLoop(*FindOuterEdge()); }

		Polygon GetOuterPolygon() const;

		class Edge : public DataOwner
		{
		public:
			Edge();
			Edge(const Edge& rhs) = delete;
			Edge(const Vert* _vert, Face* _face = nullptr, Edge* _prev = nullptr, Edge* next = nullptr, Edge* twin = nullptr);

			void Save(Kernel::Serial::SaveNode& node) const;
			void Load(const Kernel::Serial::LoadNode& node);

			Vec2 GetVec() const;
			double GetAngle() const;
			bool IsConcave() const { return GetAngle() < 0; }
			bool DoesVectorPointInside(const Jig::Vec2& vec) const;
			bool IsRedundant() const;
			bool IsConnectedTo(const Edge& edge) const;
			Line2 GetLine() const;
			const Face* GetTwinFace() const;
			EdgeMesh::Edge& GetFirstShared(); // CCW
			const EdgeMesh::Edge& GetFirstShared() const { return const_cast<Edge*>(this)->GetFirstShared(); }
			EdgeMesh::Edge* GetPrevShared(); // CCW
			const EdgeMesh::Edge* GetPrevShared() const { return const_cast<Edge*>(this)->GetPrevShared(); }
			EdgeMesh::Edge* GetNextShared(); // CW
			const EdgeMesh::Edge* GetNextShared() const { return const_cast<Edge*>(this)->GetNextShared(); }
			const Edge* FindSharedEdge(const Face& face) const;
			Edge* FindNextOuterEdge();
			const Edge* FindNextOuterEdge() const { return const_cast<Edge*>(this)->FindNextOuterEdge(); }
			Edge* FindPrevOuterEdge();
			const Edge* FindPrevOuterEdge() const { return const_cast<Edge*>(this)->FindPrevOuterEdge(); }
			Edge* FindSharedOuterEdge();
			const Edge* FindSharedOuterEdge() const { return const_cast<Edge*>(this)->FindSharedOuterEdge(); }
			void ConnectTo(Edge& edge);
			void SetTwin(Edge& edge);
			void Dump() const;

			// Doesn't rewind first - use GetSharedEdges(GetFirstShared()).
			SharedEdges GetSharedEdges() { return SharedEdges(*this); }
			ConstSharedEdges GetSharedEdges() const { return ConstSharedEdges(*this); }

			Face* face;
			const Vert* vert;
			Edge *prev, *next, *twin;
		};

		class Face : public DataOwner
		{
			friend class EdgeMesh;
			friend class Edge;
			friend void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
		public:
			Face() {}
			Face(Edge& edgeLoopToAdopt);
			Face(const Face& rhs) = delete;

			void Save(Kernel::Serial::SaveNode& node) const;
			void Load(const Kernel::Serial::LoadNode& node);

			// Low level ops - connections not updated.
			Edge& PushEdge(EdgePtr edge);
			EdgePtr PopEdge();
			std::pair<EdgePtr, size_t> RemoveEdge(Edge& edge);
			void InsertEdge(EdgePtr edge, size_t index);
			
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

			std::vector<EdgePtr>& GetEdgesUnordered() { return m_edges; } 
			int GetEdgeCount() const { return (int)m_edges.size(); }
			Polygon GetPolygon() const;
			void AssertValid() const;
			bool IsConcave() const;
			bool Contains(const Vec2& point) const;
			bool Contains(const PolyLine& poly) const;

			bool DissolveToFit(const PolyLine& poly, std::vector<Face*>& deletedFaces, std::vector<Polygon>& newHoles);

			void Update();

			void Dump() const;

		private:
			Edge & AddEdge(const Vert* vert);
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
	};
void swap(EdgeMesh::Face& lhs, EdgeMesh::Face& rhs);
}

