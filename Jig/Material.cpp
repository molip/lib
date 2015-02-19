#pragma once

#include "Material.h"

#include <SFML/OpenGL.hpp>

using namespace Jig;

Material::Material() : ambient(0.2f, 0.2f, 0.2f), diffuse(0.8f, 0.8f, 0.8f), specular(0, 0, 0), emission(0, 0, 0), shininess(0)
{
}

void Material::Apply() const
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SPECULAR, emission);
	glMateriali(GL_FRONT, GL_SHININESS, shininess);
}
