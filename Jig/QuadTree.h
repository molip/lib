#pragma once

#include "Rect.h"

#include "libKernel/Debug.h"
#include "libKernel/EnumArray.h"

#include <vector>
#include <memory>

namespace Jig
{
	template <typename T>
	class QuadTree
	{
		enum class Corner { None = -1, NW, NE, SE, SW, _Count };

		class Node
		{
		public:
			Node() {}
			Node(const Rect& r) : m_rect(r), m_centre(r.GetCentre()) {}

			void Insert(const T* t)
			{
				const Rect r = t->GetBBox();
				Corner c = Corner::None;

				if (r.m_p1.x < m_centre.x)
				{
					if (r.m_p1.y < m_centre.y)
						c = Corner::NW;
					else if (r.m_p0.y >= m_centre.y)
						c = Corner::SW;
				}
				else if (r.m_p0.x >= m_centre.x)
				{
					if (r.m_p1.y < m_centre.y)
						c = Corner::NE;
					else if (r.m_p0.y >= m_centre.y)
						c = Corner::SE;
				}

				if (c == Corner::None)
					m_items.push_back(t);
				else
				{
					if (!m_nodes[c])
					{
						Rect subRect = m_rect;
						if (c == Corner::NE || c == Corner::NW)
							subRect.m_p1.y = m_centre.y;
						else
							subRect.m_p0.y = m_centre.y;

						if (c == Corner::NW || c == Corner::SW)
							subRect.m_p1.x = m_centre.x;
						else
							subRect.m_p0.x = m_centre.x;


						m_nodes[c] = std::make_unique<Node>(subRect);
					}
					m_nodes[c]->Insert(t);
				}
			}

			const T* HitTest(const Vec2& point) const
			{
				if (!m_rect.Contains(point))
					return nullptr;

				for (auto& node : m_nodes)
					if (node)
						if (auto* t = node->HitTest(point))
							return t;

				for (auto* item : m_items)
					if (item->Contains(point))
						return item;

				return nullptr;
			}

		private:
			typedef std::unique_ptr<Node> Ptr;

			Rect m_rect;
			Vec2 m_centre;
			Kernel::EnumArray<Corner, Ptr> m_nodes;
			std::vector<const T*> m_items;
		};

	public:
		QuadTree() {}
		~QuadTree() {}

		void Reset(const Rect& rect)
		{
			m_node = Node{ rect };
		}

		void Insert(const T* t) 
		{
			m_node.Insert(t); 
		}

		const T* HitTest(const Vec2& point) const
		{
			return m_node.HitTest(point);
		}

	private:
		Node m_node;
	};
}

