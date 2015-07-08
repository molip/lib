#pragma once

#include "Vector.h"
#include "Polygon.h"

#include "libKernel/Debug.h"

#include <list>
#include <memory>
#include <vector>

namespace Jig
{
	class Vectoriser
	{
	public:
		class Sampler
		{
		public:
			virtual bool Get(int x, int y) const = 0;
		};

		using Results = std::vector<PolyPolygon>;

		Vectoriser(const Sampler& sampler, int xSamples, int ySamples);
		~Vectoriser();

		void Go();
		Results GetResults() const;

	private:
		class Shape;
		class Segment;

		class Node : std::enable_shared_from_this<Node>
		{
		public:
			using Ptr = std::unique_ptr<Node>;

			Node(int _x, int _y, Segment& _segment) : x(_x), y(_y), segment(&_segment) {}

			static Ptr Make(int x, int y, Segment& segment) { return std::make_unique<Node>(x, y, segment); }

			Node* PushFront(int _x, int _y);
			Node* PushBack(int _x, int _y);

			bool IsFront() const { return !segment->empty() && segment->front().get() == this; }
			bool IsBack() const { return !segment->empty() && segment->back().get() == this; }

			int x, y;
			Segment* segment;
		};

		class Segment : public std::list<Node::Ptr>
		{
		public:
			Segment(Shape& _shape, int first, int second, int y) : shape(&_shape)
			{
				emplace_back(Node::Make(first, y, *this));
				emplace_back(Node::Make(second, y, *this));
			}

			void SpliceBack(Segment& segment);
			bool IsOuter() const;

			Shape* shape;
		};
		using SegmentPtr = std::unique_ptr<Segment>;

		class Span : public std::pair<int, int> // Half open.
		{
		public:
			Span(int startX, int endX, int y) : pair(startX, endX), m_y(y) {}

			void AddChild(Span& span)
			{
				m_children.push_back(&span);
				span.m_parents.push_back(this);
			}

			bool HasParent() const { return !m_parents.empty(); }

			bool Touches(const Span& span) const
			{
				return second >= span.first && span.second >= first;
			}

			Node* AddPointL(bool bottom) { return leftNode->PushFront(first, m_y + bottom); }
			Node* AddPointR(bool bottom) { return rightNode->PushBack(second, m_y + bottom); }

			Shape& GetShape() { KERNEL_ASSERT(leftNode->segment->shape == rightNode->segment->shape); return *leftNode->segment->shape; }

			Shape* Apply(); // Returns merged shape for deletion.

			Node* leftNode;
			Node* rightNode;
			std::vector<Span*> m_parents;
			std::vector<Span*> m_children;
			int m_y;


		};
		using SpanPtr = std::unique_ptr<Span>;

		class Shape
		{
		public:
			Shape(Span& span)
			{
				m_segments.push_back(std::make_unique<Segment>(*this, span.first, span.second, span.m_y));
				span.leftNode = m_segments.back()->front().get();
				span.rightNode = m_segments.back()->back().get();
			}

			void AddHole(Span& left, Span& right)
			{
				KERNEL_ASSERT(left.m_y == right.m_y);
				m_segments.emplace_back(std::make_unique<Segment>(*this, right.first, left.second, left.m_y));
				right.leftNode = m_segments.back()->front().get();
				left.rightNode = m_segments.back()->back().get();
			}

			void Adopt(Shape & shape);

			PolyPolygon GetPolyPolygon() const;

			const Segment* GetOuterSegment() const;
			void MakeOuter(const Segment& segment);

		private:
			std::vector<SegmentPtr> m_segments;
		};
		using ShapePtr = std::unique_ptr<Shape>;

		std::vector<ShapePtr> m_shapes;

		const Sampler& m_sampler;
		int m_xSamples, m_ySamples;
	};
}