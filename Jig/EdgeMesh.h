#pragma once

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

		void Init(const Polygon& poly);

		typedef Vec2 Vert;
		class Edge;
		class Face;

		Face& SplitFace(Face& face, Edge& e0, Edge& e1);

		typedef std::shared_ptr<Vert> VertPtr;
		typedef std::unique_ptr<Edge> EdgePtr;
		typedef std::unique_ptr<Face> FacePtr;

		void Clear() { m_faces.clear(); }
		const std::vector<FacePtr>& GetFaces() const { return m_faces; }

		template <typename T>
		class EdgeIter
		{
		public:
			EdgeIter(T* start, bool started = false) :
				m_start(start), m_started(started), m_current(start) {}
			bool operator !=(const EdgeIter<T>& rhs) const { return m_current != rhs.m_current || m_started != rhs.m_started; }
			T& operator* () const { return *m_current; }
			void operator++ () { m_current = m_current->next; m_started = true; }
		private:
			T *m_start, *m_current;
			bool m_started;
		};

		template <typename T>
		class EdgeRange
		{
		public:
			EdgeRange(T& start) : m_begin(&start), m_end(&start, true) {}

			EdgeIter<T> begin() const { return m_begin; }
			EdgeIter<T> end() const { return m_end; }

		private:
			EdgeIter<T> m_begin, m_end;
		};

		class Edge
		{
		public:
			Edge();
			Edge(Face* _face, VertPtr _vert, Edge* _prev = nullptr, Edge* next = nullptr, Edge* twin = nullptr);

			Vec2 GetVec() const;
			double GetAngle() const;
			bool IsConcave() const { return GetAngle() < 0; }

			Face* face;
			VertPtr vert;
			Edge *prev, *next, *twin;
		};

		class Face
		{
		public:
			Face() {}
			Face(const Face& rhs) = delete;
			Face(const Face&& rhs);
			Face(const Polygon& poly);

			Edge& GetEdge() { return **m_edges.begin(); }
			const Edge& GetEdge() const { return **m_edges.begin(); }
			EdgeRange<Edge> GetEdges() { return EdgeRange<Edge>(GetEdge()); }
			EdgeRange<const Edge> GetEdges() const { return EdgeRange<const Edge>(GetEdge()); }
			int GetEdgeCount() const { return (int)m_edges.size(); }
			Polygon GetPolygon() const;
			bool IsValid() const;
			bool IsConcave() const;

			Edge& AddEdgeAfter(Edge& prev, VertPtr vert, Edge* twin = nullptr);
			FacePtr Split(Edge& e0, Edge& e1);

		private:
			std::vector<EdgePtr> m_edges; // Unordered.
		};

	private:
		void Convexify(Face& face);

		double GetAngle(const Edge& edge) const;
		Vec2 GetVec(const Edge& edge) const;

		std::vector<FacePtr> m_faces;
	};
}