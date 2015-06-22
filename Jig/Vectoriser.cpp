#include "Vectoriser.h"

using namespace Jig;

Vectoriser::Vectoriser(const Sampler& sampler, const int xSamples, int ySamples) : m_sampler(sampler), m_xSamples(xSamples), m_ySamples(ySamples)
{
}

Vectoriser::~Vectoriser()
{
}

void Vectoriser::Go()
{
	using Line = std::vector<SpanPtr>;

	Line lastLine;
	for (int y = 0; y < m_ySamples; ++y)
	{
		Line thisLine;
		thisLine.reserve(1024);
		auto parent = lastLine.begin();
		for (int x = 0; x < m_xSamples; ++x)
		{
			if (m_sampler.Get(x, y))
			{
				int start = x;
				while (++x < m_xSamples && m_sampler.Get(x, y));
				int end = x;

				thisLine.push_back(std::make_unique<Span>(start, end, y));
				auto& span = thisLine.back();

				while (parent != lastLine.end() && (*parent)->second < span->first) // Skip earlier potential parents. 
				{
					++parent;
				}

				while (parent != lastLine.end() && (*parent)->first <= span->second)
				{
					(*parent++)->AddChild(*span);
				}

				if (span->HasParent()) // Reconsider last parent for next span. 
				{
					--parent;
				}
				else // New shape. 
				{
					m_shapes.push_back(std::make_unique<Shape>(*span));
				}
			}
		}

		for (auto& span : lastLine)
		{
			span->Apply();
		}

		lastLine = std::move(thisLine);
	}

	for (auto& span : lastLine)
	{
		span->Apply();
	}
}

Vectoriser::Results Vectoriser::GetResults() const
{
	Results results;
	results.reserve(m_shapes.size());

	for (auto& shape : m_shapes)
	{
		auto pp = shape->GetPolyPolygon();
		if (!pp.empty())
			results.push_back(pp);
	}
	return results;

}

void Vectoriser::Shape::Adopt(Shape& shape)
{
	m_segments.reserve(m_segments.size() + shape.m_segments.size());
	for (auto& seg : shape.m_segments)
	{
		if (!seg->empty()) // Skip the dead one we just spliced from.
		{
			seg->shape = this;
			m_segments.push_back(std::move(seg));
		}
	}
}

PolyPolygon Vectoriser::Shape::GetPolyPolygon() const
{
	PolyPolygon pp;
	pp.reserve(m_segments.size());

	for (auto& seg : m_segments)
	{
		if (seg && !seg->empty())
		{
			pp.emplace_back();
			auto& poly = pp.back();
			poly.reserve(seg->size());
			for (auto& node : *seg)
			{
				poly.emplace_back(node->x, node->y);
			}
		}
	}
	return pp;
}

Vectoriser::Shape* Vectoriser::Span::Apply()
{
	Shape* deadShape = nullptr;

	leftNode = AddPointL(true);
	rightNode = AddPointR(true);

	if (m_children.empty()) // Bottom of a segment. 
	{
		if (leftNode->segment != rightNode->segment) // Else finished shape. 
		{
			rightNode->segment->SpliceBack(*leftNode->segment);
		}
		return nullptr;
	}

	Span& firstChild = *m_children.front();
	if (firstChild.m_parents.front() == this) // Continue outer segment. 
	{
		//          this
		// firstChild

		firstChild.leftNode = leftNode->PushFront(firstChild.first, firstChild.m_y);
	}
	else // Connect to firstChild's previous parent.
	{
		// prev        this
		//    firstChild  

		Segment& prevSegment = *firstChild.rightNode->segment; // Actually firstChild's previous parent's rightNode.
		firstChild.rightNode = nullptr;
		if (&prevSegment != leftNode->segment) // Append segment. Else we've just finished a hole. 
		{
			Shape& thisShape = GetShape();
			Shape& prevShape = *prevSegment.shape;

			prevSegment.SpliceBack(*leftNode->segment);

			if (&thisShape != &prevShape)
			{
				prevShape.Adopt(thisShape);
				deadShape = &thisShape;
			}
		}
	}

	for (int i = 1; i < m_children.size(); ++i) // Start new segments, which might turn out to be holes (but not if they later join the outer segment).
	{
		//          this_this_this_this
		// firstChild   nextChild     lastChild

		GetShape().AddHole(*m_children[i - 1], *m_children[i]);
	}

	Span& lastChild = *m_children.back();
	if (m_children.back()->m_parents.back() == this)  // Continue outer segment.
	{
		// this
		//    lastChild

		lastChild.rightNode = rightNode->PushBack(lastChild.second, lastChild.m_y);
	}
	else
	{
		// this       next
		//    lastChild

		lastChild.rightNode = rightNode; // Temporarily store rightNode here for next span to connect to.
	}

	for (auto& child : m_children)
	{
		KERNEL_ASSERT(child->leftNode->IsFront() && child->rightNode->IsBack());
		KERNEL_ASSERT(child->leftNode && child->rightNode);
		KERNEL_ASSERT(child->leftNode->segment && child->rightNode->segment);
	}

	return deadShape;
}

Vectoriser::Node* Vectoriser::Node::PushFront(int _x, int _y)
{
	if (_x == x && _y == y)
		return this;

	KERNEL_ASSERT(IsFront());
	segment->push_front(Make(_x, _y, *segment));
	Node* node = segment->front().get();
	return node;
}

Vectoriser::Node* Vectoriser::Node::PushBack(int _x, int _y)
{ 
	if (_x == x && _y == y)
		return this;

	KERNEL_ASSERT(IsBack());
	segment->push_back(Make(_x, _y, *segment));
	Node* node = segment->back().get();
	return node;
}

void Vectoriser::Segment::SpliceBack(Segment& segment)
{
	KERNEL_ASSERT(&segment != this);

	if (!segment.empty())
	{
		segment.back()->segment = this;
		segment.front()->segment = this;
	}

	splice(end(), segment);
}
