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
		EdgeMesh::Edge* GetNewEdge() { return m_items[0].newEdge.get(); }

	private:
		void AssertFacesValid() const;

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

	class AddFace : public Base
	{
	public:
		const std::vector<Jig::EdgeMesh::VertPtr>& GetNewVerts() const { return m_newVerts; }

	protected:
		AddFace(EdgeMesh& mesh);

		EdgeMesh& m_mesh;
		EdgeMesh::FacePtr m_face;
		std::vector<Jig::EdgeMesh::VertPtr> m_newVerts;
	};
	
	class AddOuterFace : public AddFace
	{
	public:
		AddOuterFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points);
		virtual void Do() override;
		virtual void Undo() override;

	private:
		void Init(Jig::EdgeMesh::Edge& oldStart, Jig::EdgeMesh::Edge& oldEnd, const PolyLine& points);
			
		std::vector<Jig::EdgeMesh::Edge*> m_oldEdges;
	};

	class SplitFace : public AddFace
	{
	public:
		SplitFace(EdgeMesh& mesh, Jig::EdgeMesh::Edge& start, Jig::EdgeMesh::Edge& end, const PolyLine& points);
		virtual void Do() override;
		virtual void Undo() override;

	private:
		void CreateNewEdgesForOldFace(const PolyLine& points);
		void CreateNewFace();
			
		Jig::EdgeMesh::Edge& m_start;
		Jig::EdgeMesh::Edge& m_end;
		std::vector<Jig::EdgeMesh::EdgePtr> m_newEdges, newTwins;
		std::vector<size_t> m_oldEdgePositions;
	};

	class MoveVert : public Base
	{
	public:
		MoveVert(EdgeMesh::Vert& vert, const Vec2& pos);
		virtual void Do() override;
		virtual void Undo() override;

	private:
		EdgeMesh::Vert& m_vert;
		Vec2 m_pos;
	};
}
