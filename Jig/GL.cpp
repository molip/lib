#include "GL.h"

#include "Vector.h"

#include <SFML/OpenGL.hpp>

using namespace Jig;

void GL::Translate(const Vec3& vec)
{
	glTranslated(vec.x, vec.y, vec.z);
}

void GL::Rotate(double angle, const Vec3& vec)
{
	glRotated(angle, vec.x, vec.y, vec.z);
}

void GL::Vertex(const Vec3& vec)
{
	glVertex3d(vec.x, vec.y, vec.z);
}

void GL::Normal(const Vec3& vec)
{
	glNormal3d(vec.x, vec.y, vec.z);
}
