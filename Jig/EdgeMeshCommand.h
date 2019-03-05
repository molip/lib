#pragma once

#include "EdgeMesh.h"

namespace Jig::EdgeMeshCommand
{
	class Base
	{
	public:
		virtual ~Base();
		virtual bool CanDo() const { return true; }
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

		virtual bool CanDo() const override;
		virtual void Do() override;
		virtual void Undo() override;

		std::vector<std::unique_ptr<Base>> m_children;
	};

	class InsertVerts : public Base
	{
	public:
		InsertVerts(EdgeMesh& mesh, EdgeMesh::Edge& edge, const Vec2& pos);
		InsertVerts(EdgeMesh& mesh, EdgeMesh::Edge& edge, std::initializer_list<Vec2> positions);
		virtual void Do() override;
		virtual void Undo() override;

		const std::vector<EdgeMesh::VertPtr>& GetVerts() { return m_newVerts; }

	private:
		void AssertFacesValid() const;

		struct Item
		{
			Item(EdgeMesh::Edge* oldEdge) : oldEdge(oldEdge) {}
			EdgeMesh::Edge* oldEdge{};
			std::vector<EdgeMesh::EdgePtr> newEdges;
		};

		std::vector<Item> m_items;
		EdgeMesh& m_mesh;
		std::vector<EdgeMesh::VertPtr> m_newVerts;
		//std::vector<const EdgeMesh::Vert*> m_newVertPtrs;
	};

	class DeleteVert : public Base
	{
	public:
		DeleteVert(EdgeMesh& mesh, EdgeMesh::Vert& vert);

		virtual bool CanDo() const override;
		virtual void Do() override;
		virtual void Undo() override;

		std::vector<const EdgeMesh::Edge*> GetDeletedEdges() const { return m_deletedEdges; }

	private:
		struct Item
		{
			Item(EdgeMesh::EdgePtr oldEdge, size_t oldIndex) : oldEdge(std::move(oldEdge)), oldIndex(oldIndex) {}
			EdgeMesh::EdgePtr oldEdge;
			size_t oldIndex{};
			EdgeMesh::Edge* oldPrevTwin{};
		};

		EdgeMesh& m_mesh;
		Jig::EdgeMesh::Vert& m_vert;
		EdgeMesh::Edge* m_startEdge;
		EdgeMesh::VertPtr m_oldVert;
		size_t m_oldVertIndex{};
		std::vector<Item> m_items;
		std::vector<const EdgeMesh::Edge*> m_deletedEdges;
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

	class MergeFace : public Base
	{
	public:
		MergeFace(EdgeMesh& mesh, EdgeMesh::Edge& edge); // Edge twinned with face to merge to. 

		virtual bool CanDo() const override;
		virtual void Do() override;
		virtual void Undo() override;

		std::vector<const EdgeMesh::Vert*> GetDeletedVerts() const { return m_deletedVerts; }
		std::vector<const EdgeMesh::Edge*> GetDeletedEdges() const { return m_deletedEdges; }

	private:
		EdgeMesh& m_mesh;
		EdgeMesh::Edge& m_edge;
		EdgeMesh::Edge* m_first;
		EdgeMesh::Edge* m_last;

		using EdgeItem = std::pair<EdgeMesh::EdgePtr, size_t>;
		using EdgeItemPair = std::pair<EdgeItem, EdgeItem>;
		using FaceItem = std::pair<EdgeMesh::FacePtr, size_t>;
		using VertItem = std::pair<EdgeMesh::VertPtr, size_t>;

		std::vector<EdgeItemPair> m_deleted;
		std::vector<size_t> m_adopted;
		std::vector<VertItem> m_oldVerts;
		std::vector<const EdgeMesh::Vert*> m_deletedVerts;
		std::vector<const EdgeMesh::Edge*> m_deletedEdges;
		FaceItem m_oldFace;
	};

	class DeleteFace : public Base
	{
	public:
		DeleteFace(EdgeMesh& mesh, EdgeMesh::Face& face);

		virtual bool CanDo() const override;
		virtual void Do() override;
		virtual void Undo() override;

		std::vector<const EdgeMesh::Vert*> GetDeletedVerts() const { return m_deletedVerts; }

	private:
		EdgeMesh & m_mesh;
		EdgeMesh::Face& m_face;

		using FaceItem = std::pair<EdgeMesh::FacePtr, size_t>;
		using VertItem = std::pair<EdgeMesh::VertPtr, size_t>;

		std::vector<VertItem> m_oldVerts;
		std::vector<const EdgeMesh::Vert*> m_deletedVerts;
		std::vector<EdgeMesh::Edge*> m_oldTwins;
		FaceItem m_oldFace;
	};
}
