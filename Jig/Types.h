#include "Vector.h"

#include <SFML/Graphics/Rect.hpp>

namespace Jig
{
	//typedef sf::IntRect Rect;
	typedef sf::Vector2i Point2;
	typedef sf::Vector3i Point3;
	typedef sf::Vector2i Size2;
	typedef sf::Vector3i Size3;
}

namespace Kernel
{
	namespace Serial
	{
		class SaveNode;
		class LoadNode;
	}
}