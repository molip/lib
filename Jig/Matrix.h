#pragma once

namespace Jig
{
	class Vec3;
	class Matrix
	{
	public:
		void MakeIdentity();
		void MakeRotationX(float angle);
		void MakeRotationY(float angle);
		void MakeRotationZ(float angle);
		void MakeRotationYXZ(float ry, float rx, float rz);
		void MakeScale(float sx, float sy, float sz);
		//void MakeZAxisToVector( const Vector& vec );
		void MakeInverse(const Matrix& src);
		//	void MakeRotationVector(float angle, const Vector& axis);

		void MultRight(const Matrix& a);
		void MultLeft(const Matrix& a);

		void MultPoint(Vec3& dst) const;
		void MultPoint(Vec3& dst, const Vec3& src) const;
		void MultPointInverse(Vec3& dst) const;
		void MultPointInverse(Vec3& dst, const Vec3& src) const;

		float* GetData() { return mat; }
		const float* GetData() const { return mat; }

		//	void GetZAxis( Vector& zVec ) const; 

		void Print() const;
		void Get4x4(float m[16]) const;
		void glMultMatrix() const;

	private:
		static void Mult(Matrix* pDst, const Matrix& a, const Matrix& b);
		float mat[9];

		float Clip(float a);
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