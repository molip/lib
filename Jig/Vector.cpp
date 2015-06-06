#include "Vector.h"

std::ostream& Jig::operator<<(std::ostream& stream, const Vec2& val)
{
	stream << "(" << val.x << "," << val.y << ")";

	return stream;
}

std::istream& Jig::operator>>(std::istream& stream, Vec2& val)
{
	char lparen, comma, rparen;
	double x, y;
	stream >> lparen >> x >> comma >> y >> rparen;

	if (lparen == '(' && comma == ',' && rparen == ')')
		val.x = x, val.y = y;

	return stream;
}
