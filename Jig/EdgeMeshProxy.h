#pragma once

#include "EdgeMesh.h"

#include <unordered_map>

namespace Jig
{
	namespace Serial = Kernel::Serial;

	class EdgeMeshProxy
	{
	public:
		EdgeMeshProxy() {}
		EdgeMeshProxy(const EdgeMesh& obj);

		void Save(Serial::SaveNode& node) const;
		void Load(const Serial::LoadNode& node, EdgeMesh& mesh);

	private:
		class SaveContext
		{
		public:
			SaveContext(const EdgeMesh& mesh);

			std::unordered_map<const EdgeMesh::Vert*, int> vertMap;
			std::unordered_map<const EdgeMesh::Edge*, int> edgeMap;
		};

		class LoadContext
		{
		public:
			std::vector<EdgeMesh::Vert*> verts;
			std::vector<EdgeMesh::Edge*> edges;
		};

		class DataOwner
		{
		public:
			DataOwner() {}
			DataOwner(const EdgeMesh::DataOwner& obj);
			~DataOwner() { delete m_data; }

			void Save(Serial::SaveNode& node) const;
			void Load(const Serial::LoadNode& node);

			EdgeMesh::Data* m_data{};
		};

		class Vert : public DataOwner
		{
		public:
			Vert() {}
			Vert(const EdgeMesh::Vert& obj);

			void Save(Serial::SaveNode& node) const;
			void Load(const Serial::LoadNode& node);

			EdgeMesh::VertPtr CreateVert();

			Vec2 m_position;
		};

		class Edge : public DataOwner
		{
		public:
			Edge() {}
			Edge(const EdgeMesh::Edge& obj, SaveContext& ctx);

			void Save(Serial::SaveNode& node) const;
			void Load(const Serial::LoadNode& node);

			int m_vert = -1, m_prev = -1, m_next = -1, m_twin = -1;
		};

		class Face : public DataOwner
		{
		public:
			Face() {}
			Face(const EdgeMesh::Face& obj, SaveContext& ctx);

			void Save(Serial::SaveNode& node) const;
			void Load(const Serial::LoadNode& node);

			std::vector<Edge> m_edges;
		};

		std::vector<Vert> m_verts;
		std::vector<Face> m_faces;
	};
}
