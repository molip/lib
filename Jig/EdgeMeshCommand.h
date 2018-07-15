#pragma once

#include "EdgeMesh.h"

namespace Jig::EdgeMeshCommand
{
	class Base
	{
	public:
		virtual ~Base();
		virtual void Do() = 0;
		virtual void Undo() = 0;
	};

	class Compound : public Base
	{
	public:
		using Vec = std::vector<std::unique_ptr<Base>>;
		Compound();
		Compound(Vec&& children);

		void AddChild(std::unique_ptr<Base>);

		virtual void Do() override;
		virtual void Undo() override;

		std::vector<std::unique_ptr<Base>> m_children;
	};

	class InsertVert : public Base
	{
	public:
		InsertVert(Jig::EdgeMesh& mesh, Jig::EdgeMesh::Edge& edge, const Jig::Vec2& pos);
		virtual void Do() override;
		virtual void Undo() override;

		EdgeMesh::Vert* GetVert() { return m_newVert.get(); }

	private:
		struct Item
		{
			Item(Jig::EdgeMesh::Edge* oldEdge) : oldEdge(oldEdge) {}
			Jig::EdgeMesh::Edge* oldEdge{};
			EdgeMesh::EdgePtr newEdge;
		};

		std::vector<Item> m_items;

		EdgeMesh& m_mesh;
		const Jig::Vec2 m_pos;

		EdgeMesh::VertPtr m_newVert;
	};

	class AddOuterFace : public Base
	{
	public:
		AddOuterFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points);
		virtual void Do() override;
		virtual void Undo() override;

	private:
		void Init(Jig::EdgeMesh::Edge& oldStart, Jig::EdgeMesh::Edge& oldEnd, const PolyLine& points);
			
		EdgeMesh& m_mesh;
		EdgeMesh::FacePtr m_face;
		std::vector<Jig::EdgeMesh::Edge*> m_oldEdges;
		std::vector<Jig::EdgeMesh::VertPtr> m_newVerts;
	};

	class SplitFace : public Base
	{
	public:
		SplitFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points);
		virtual void Do() override;
		virtual void Undo() override;

	private:
		void CreateNewEdgesForOldFace(const PolyLine& points);
		void CreateNewFace();
			
		EdgeMesh& m_mesh;
		Jig::EdgeMesh::Edge& m_start;
		Jig::EdgeMesh::Edge& m_end;
		EdgeMesh::FacePtr m_face;
		std::vector<Jig::EdgeMesh::EdgePtr> m_newEdges, newTwins;
		std::vector<Jig::EdgeMesh::VertPtr> m_newVerts;
		std::vector<size_t> m_oldEdgePositions;
	};
}
