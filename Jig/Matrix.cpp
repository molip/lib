#include "matrix.h"
#include "Vector.h"

#include <iostream>
#include <SFML/OpenGL.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;
using namespace Jig;

void Matrix::MakeIdentity()
{
	mat[0] = 1; mat[1] = 0; mat[2] = 0;
	mat[3] = 0; mat[4] = 1; mat[5] = 0;
	mat[6] = 0; mat[7] = 0; mat[8] = 1;
}

void Matrix::MultRight( const Matrix& b )
{
	Matrix a = *this;
	Mult( this, a, b );
}

// this is the normal one, ie. this = this * a
void Matrix::MultLeft( const Matrix& a )
{
	Matrix b = *this;
	Mult( this, a, b );
}

// assumes pDst != &a && pDst != &b
void Matrix::Mult( Matrix* pDst, const Matrix& a, const Matrix& b ) // static
{
	pDst->mat[0] = a.mat[0]*b.mat[0] + a.mat[1]*b.mat[3] + a.mat[2]*b.mat[6];
	pDst->mat[1] = a.mat[0]*b.mat[1] + a.mat[1]*b.mat[4] + a.mat[2]*b.mat[7];
	pDst->mat[2] = a.mat[0]*b.mat[2] + a.mat[1]*b.mat[5] + a.mat[2]*b.mat[8];

	pDst->mat[3] = a.mat[3]*b.mat[0] + a.mat[4]*b.mat[3] + a.mat[5]*b.mat[6];
	pDst->mat[4] = a.mat[3]*b.mat[1] + a.mat[4]*b.mat[4] + a.mat[5]*b.mat[7];
	pDst->mat[5] = a.mat[3]*b.mat[2] + a.mat[4]*b.mat[5] + a.mat[5]*b.mat[8];

	pDst->mat[6] = a.mat[6]*b.mat[0] + a.mat[7]*b.mat[3] + a.mat[8]*b.mat[6];
	pDst->mat[7] = a.mat[6]*b.mat[1] + a.mat[7]*b.mat[4] + a.mat[8]*b.mat[7];
	pDst->mat[8] = a.mat[6]*b.mat[2] + a.mat[7]*b.mat[5] + a.mat[8]*b.mat[8];
}

void Matrix::MultPoint( Vec3& vec ) const
{
	Vec3 dst;
	MultPoint( dst, vec );
	vec = dst;
}

// assumes &dst != &src
void Matrix::MultPoint( Vec3& dst, const Vec3& src ) const
{
	dst.x = mat[0]*src.x + mat[3]*src.y + mat[6]*src.z;
	dst.y = mat[1]*src.x + mat[4]*src.y + mat[7]*src.z;
	dst.z = mat[2]*src.x + mat[5]*src.y + mat[8]*src.z;
}

// assumes orthogonal
void Matrix::MultPointInverse( Vec3& vec ) const
{
	Vec3 dst;
	MultPointInverse( dst, vec );
	vec = dst;
}

// assumes orthogonal and no translation
// assumes &dst != &src
void Matrix::MultPointInverse( Vec3& dst, const Vec3& src ) const
{
	dst.x = mat[0]*src.x + mat[1]*src.y + mat[2]*src.z;
	dst.y = mat[3]*src.x + mat[4]*src.y + mat[5]*src.z;
	dst.z = mat[6]*src.x + mat[7]*src.y + mat[8]*src.z;
}

void Matrix::MakeRotationX( double angle )
{
	double s = sin( angle );
	double c = cos( angle );

	mat[0] = 1; mat[3] = 0; mat[6] = 0;
	mat[1] = 0; mat[4] = c; mat[7] = -s;
	mat[2] = 0; mat[5] = s; mat[8] = c;
}

void Matrix::MakeRotationY( double angle )
{
	double s = sin( angle );
	double c = cos( angle );

	mat[0] = c; mat[3] = 0; mat[6] = s;
	mat[1] = 0; mat[4] = 1; mat[7] = 0;
	mat[2] = -s; mat[5] = 0; mat[8] = c;
}

void Matrix::MakeRotationZ( double angle )
{
	double s = sin( angle );
	double c = cos( angle );

	mat[0] = c; mat[3] = -s; mat[6] = 0;
	mat[1] = s; mat[4] = c; mat[7] = 0;
	mat[2] = 0; mat[5] = 0; mat[8] = 1;
}

void Matrix::MakeRotationYXZ( double ry, double rx, double rz )
{
    double A = cos( rx );
    double B = sin( rx );
    double C = cos( ry );
    double D = sin( ry );
    double E = cos( rz );
    double F = sin( rz );
	double BD = B*D;

	mat[0] = E*C + F*BD;
	mat[1] = F*A;
	mat[2] = -E*D + F*B*C;
	mat[3] = -F*C + E*BD;
	mat[4] = E*A;
	mat[5] = +F*D +E*B*C;
	mat[6] = A*D;
	mat[7] = -B;
	mat[8] = A*C;

}
void Matrix::MakeScale( double sx, double sy, double sz )
{
	mat[0] = sx; mat[1] = 0; mat[2] = 0;
	mat[3] = 0; mat[4] = sy; mat[5] = 0;
	mat[6] = 0; mat[7] = 0; mat[8] = sz;
}

//// make a matrix to rotate vector (0, 0, -1) to vec
//void Matrix::MakeZAxisToVector( const Vector& vec )
//{
//	double hypXZ = sqrt( vec.x * vec.x + vec.z * vec.z );
//
//	// rotate round Y so x==0
//	double rotY = -atan2(vec.x, -vec.z);
//	// rotate round X so x==0, y==0
//	double rotX = ::atan2(vec.y, hypXZ);
//
//	Matrix matY;
//	matY.MakeRotationY( rotY );
//	MakeRotationX( rotX );
//	MultRight( matY );
//}
//
//void Matrix::MakeRotationVector(double angle, const Vector& axis)
//{
//	Matrix toVec, rot;
//	toVec.MakeZAxisToVector(axis);
//	rot.MakeRotationZ(-angle);
//	MakeInverse(toVec);
//
//	MultLeft(rot);
//	MultLeft(toVec);
//}

// assumes orthogonal and no translation
// assumes &dst != &src
void Matrix::MakeInverse( const Matrix& src )
{
	mat[0] = src.mat[0]; mat[1] = src.mat[3]; mat[2] = src.mat[6];
	mat[3] = src.mat[1]; mat[4] = src.mat[4]; mat[5] = src.mat[7];
	mat[6] = src.mat[2]; mat[7] = src.mat[5]; mat[8] = src.mat[8];
}

//void Matrix::GetZAxis( Vector& zVec ) const
//{
//	zVec.Set( mat[6], mat[7], mat[8] );
//}

void Matrix::Print() const
{
	cout << mat[0] << '\t' << mat[3] << '\t' << mat[6] << endl;
	cout << mat[1] << '\t' << mat[4] << '\t' << mat[7] << endl;
	cout << mat[2] << '\t' << mat[5] << '\t' << mat[8] << endl;
}

void Matrix::Get4x4( double m[16] ) const
{
	m[0] = mat[0]; m[1] = mat[1]; m[2] = mat[2]; m[3] = 0;
	m[4] = mat[3]; m[5] = mat[4]; m[6] = mat[5]; m[7] = 0;
	m[8] = mat[6]; m[9] = mat[7]; m[10] = mat[8]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void Matrix::glMultMatrix() const
{
	double m[16];
	Get4x4( m );
	glMultMatrixd( m );
}

//------------------------------

void MatrixGL::GetModelView()
{
	glGetDoublev(GL_MODELVIEW_MATRIX, m);
}

void MatrixGL::Load() const
{
	glLoadMatrixd(m);
}

void MatrixGL::Mult() const
{
	glMultMatrixd(m);
}

void MatrixGL::RemoveRotation()
{
	m[0] = 1; m[4] = 0; m[8] = 0;
	m[1] = 0; m[5] = 1; m[9] = 0;
	m[2] = 0; m[6] = 0; m[10] = 1;
}

void MatrixGL::RemoveTranslation()
{
	m[12] = m[13] = m[14] = 0;		
//	m[3] = m[7] = m[11] = 0;		
}

Vec3 MatrixGL::MultPoint(const Vec3& v)
{
	Vec3 v2;

	v2.x = double(m[0] * v.x + m[4] * v.y + m[8]  * v.z  + m[12]);
	v2.y = double(m[1] * v.x + m[5] * v.y + m[9]  * v.z  + m[13]);
	v2.z = double(m[2] * v.x + m[6] * v.y + m[10] * v.z  + m[14]);

	return v2;
}
