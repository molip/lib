#include "Texture.h"

#include <map>
#include <memory>

namespace 
{
	std::map<std::string, std::unique_ptr<sf::Texture>> map;
}

using namespace Jig::Texture;

const sf::Texture& Get(const char* sPath)
{
	auto it = map.find(sPath);
	if (it != map.end())
		return *it->second.get();

	auto p = new sf::Texture;
	if (!p->loadFromFile(sPath))
		throw;

	map.insert(std::make_pair(sPath, std::unique_ptr<sf::Texture>(p)));
	return *p;
}
