#pragma once

#include "VectorFwd.h"

namespace Jig
{
	class Matrix
	{
	public:
		void MakeIdentity();
		void MakeRotationX(double angle);
		void MakeRotationY(double angle);
		void MakeRotationZ(double angle);
		void MakeRotationYXZ(double ry, double rx, double rz);
		void MakeScale(double sx, double sy, double sz);
		//void MakeZAxisToVector( const Vector& vec );
		void MakeInverse(const Matrix& src);
		//	void MakeRotationVector(double angle, const Vector& axis);

		void MultRight(const Matrix& a);
		void MultLeft(const Matrix& a);

		void MultPoint(Vec3& dst) const;
		void MultPoint(Vec3& dst, const Vec3& src) const;
		void MultPointInverse(Vec3& dst) const;
		void MultPointInverse(Vec3& dst, const Vec3& src) const;

		double* GetData() { return mat; }
		const double* GetData() const { return mat; }

		//	void GetZAxis( Vector& zVec ) const; 

		void Print() const;
		void Get4x4(double m[16]) const;
		void glMultMatrix() const;

	private:
		static void Mult(Matrix* pDst, const Matrix& a, const Matrix& b);
		double mat[9];

		double Clip(double a);
	};

	class MatrixGL
	{
	public:
		void GetModelView();
		void Load() const;
		void Mult() const;
		void RemoveTranslation();
		void RemoveRotation();
		Vec3 MatrixGL::MultPoint(const Vec3& v);
	private:
		double m[16];
	};
}