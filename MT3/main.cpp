#define _USE_MATH_DEFINES
#include <Novice.h>
#include "Vector3.h"
#include <cmath>
#include "Matrix4x4.h"
#include "imgui.h"
#include <algorithm>

const char kWindowTitle[] = "GC2B_05_ジョ_シセイ";

struct Line {
	Vector3 origin; //!<始点
	Vector3 diff; //!<方向ベクトル
};

struct Ray {
	Vector3 origin; //!<始点
	Vector3 diff; //!<方向ベクトル
};

struct Segment {
	Vector3 origin; //!<始点
	Vector3 diff; //!<方向ベクトル
};

struct Sphere {
	Vector3 center;
	float radius;
};

struct Plane {
	Vector3 normal; //!<法線ベクトル
	float distance; //!<平面の方程式 Ax + By + Cz + D = 0 の D
};

/// <summary>
/// 透視投影行列を作成
/// </summary>
/// <param name="fovY">垂直視野角(ラジアン)</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">カメラからの距離</param>
/// <param name="farClip">カメラからの距離</param>
/// <returns>透視投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 matrix = {};
	float f = 1.0f / tanf(fovY / 2.0f);
	matrix.m[0][0] = f / aspectRatio;
	matrix.m[1][1] = f;
	matrix.m[2][2] = farClip / (farClip - nearClip);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][2] = (farClip * nearClip) / (nearClip - farClip);
	return matrix;
}

/// <summary>
/// 正射影行列を作成
/// </summary>
/// <param name="left">左端の位置</param>
/// <param name="top">上端の位置</param>
/// <param name="right">右端の位置</param>
/// <param name="bottom">下端の位置</param>
/// <param name="nearClip">カメラからの距離</param>
/// <param name="farClip">カメラからの距離</param>
/// <returns>正射影行列</returns>
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 matrix = {};
	matrix.m[0][0] = 2.0f / (right - left);
	matrix.m[1][1] = 2.0f / (top - bottom);
	matrix.m[2][2] = 1.0f / (farClip - nearClip);
	matrix.m[3][0] = (right + left) / (left - right);
	matrix.m[3][1] = (top + bottom) / (bottom - top);
	matrix.m[3][2] = nearClip / (nearClip - farClip);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

/// <summary>
/// ビューポート変換行列を作成
/// </summary>
/// <param name="left">左端の位置</param>
/// <param name="top">右端の位置</param>
/// <param name="width">長さ</param>
/// <param name="height">高さ</param>
/// <param name="minDepth">カメラからの距離</param>
/// <param name="maxDepth">カメラからの距離</param>
/// <returns>ビューポート変換行列</returns>
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 matrix = {};
	matrix.m[0][0] = width / 2.0f;
	matrix.m[1][1] = -height / 2.0f;
	matrix.m[2][2] = maxDepth - minDepth;
	matrix.m[3][0] = left + width / 2.0f;
	matrix.m[3][1] = top + height / 2.0f;
	matrix.m[3][2] = minDepth;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

/// <summary>
/// 2つのベクトルの外積を計算
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>外積の結果</returns>
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

/// <summary>
/// 平行移動行列を作成
/// </summary>
/// <param name="translate">三次元ベクトル</param>
/// <returns>平行移動行列</returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		translate.x, translate.y, translate.z, 1.0f
	};
	return matrix;
}

/// <summary>
/// スケーリング行列を作成
/// </summary>
/// <param name="scale">三次元ベクトル</param>
/// <returns>スケーリング行列</returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 matrix = {
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}

/// <summary>
/// 4x4行列の逆行列を求める
/// </summary>
/// <param name="m">4x4行列</param>
/// <returns>逆行列</returns>
Matrix4x4 Inverse(const Matrix4x4& m)
{
	Matrix4x4 result = {};
	float det = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][1] * m.m[2][3] * m.m[3][2] - m.m[1][2] * m.m[2][1] * m.m[3][3]) / det;
	result.m[0][1] = -(m.m[0][1] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][1] - m.m[0][1] * m.m[2][3] * m.m[3][2] - m.m[0][2] * m.m[2][1] * m.m[3][3]) / det;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][1] * m.m[1][3] * m.m[3][2] - m.m[0][2] * m.m[1][1] * m.m[3][3]) / det;
	result.m[0][3] = -(m.m[0][1] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][1] - m.m[0][1] * m.m[1][3] * m.m[2][2] - m.m[0][2] * m.m[1][1] * m.m[2][3]) / det;
	result.m[1][0] = -(m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][0] - m.m[1][0] * m.m[2][3] * m.m[3][2] - m.m[1][2] * m.m[2][0] * m.m[3][3]) / det;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][0] * m.m[2][3] * m.m[3][2] - m.m[0][2] * m.m[2][0] * m.m[3][3]) / det;
	result.m[1][2] = -(m.m[0][0] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][0] - m.m[0][0] * m.m[1][3] * m.m[3][2] - m.m[0][2] * m.m[1][0] * m.m[3][3]) / det;
	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][0] * m.m[1][3] * m.m[2][2] - m.m[0][2] * m.m[1][0] * m.m[2][3]) / det;
	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1]
		- m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) / det;
	result.m[2][1] = -(m.m[0][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][1]
		- m.m[0][3] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][1]) / det;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1]
		- m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) / det;
	result.m[2][3] = -(m.m[0][0] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][1]
		- m.m[0][3] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][1]) / det;
	result.m[3][0] = -(m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[1][1] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[1][2] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][2] - m.m[1][0] * m.m[2][2] * m.m[3][1]) / det;
	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) / det;
	result.m[3][2] = -(m.m[0][0] * m.m[1][1] * m.m[3][2] + m.m[0][1] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][2] - m.m[0][0] * m.m[1][2] * m.m[3][1]) / det;
	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) / det;

	return result;
}

/// <summary>
/// 同次座標系を変換した三次元ベクトルをメトリクスと掛け算して変換
/// </summary>
/// <param name="vector">三次元ベクトル</param>
/// <param name="matrix">メトリクス</param>
/// <returns>掛け算の結果</returns>
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	if (w != 0.0f) {
		result.x = (vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0]) / w;
		result.y = (vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1]) / w;
		result.z = (vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2]) / w;
	}
	else {
		result.x = 0.0f;
		result.y = 0.0f;
		result.z = 0.0f;
	}
	return result;
}

/// <summary>
/// 行列の掛け算
/// </summary>
/// <param name="x">行列x</param>
/// <param name="y">行列y</param>
/// <returns>掛け算結果</returns>
Matrix4x4 Muiltiply(const Matrix4x4& x, const Matrix4x4& y) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = x.m[i][0] * y.m[0][j] + x.m[i][1] * y.m[1][j] +
				x.m[i][2] * y.m[2][j] + x.m[i][3] * y.m[3][j];
		}
	}
	return result;
}

/// <summary>
/// 回転行列を作成
/// </summary>
/// <param name="rotate">三次元ベクトル</param>
/// <returns>回転行列</returns>
Matrix4x4 MakeRotateMatrix(const Vector3& rotate) {
	// X軸回転行列
	Matrix4x4 rotateX = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosf(rotate.x), sinf(rotate.x), 0.0f,
		0.0f, -sinf(rotate.x), cosf(rotate.x), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	// Y軸回転行列
	Matrix4x4 rotateY = {
		cosf(rotate.y), 0.0f, -sinf(rotate.y), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sinf(rotate.y), 0.0f, cosf(rotate.y), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	// Z軸回転行列
	Matrix4x4 rotateZ = {
		cosf(rotate.z), sinf(rotate.z), 0.0f, 0.0f,
		-sinf(rotate.z), cosf(rotate.z), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return Muiltiply(Muiltiply(rotateX, rotateY), rotateZ);
}

/// <summary>
/// アフィン変換
/// </summary>
/// <param name="scale">スクロールベクトル</param>
/// <param name="rotate">回転ベクトル</param>
/// <param name="translate">平行移動ベクトル</param>
/// <returns>アフィン変換マトリックス</returns>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result = {};
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	result = Muiltiply(Muiltiply(scaleMatrix, rotateMatrix), translateMatrix);
	return result;
}

/// <summary>
/// ベクトルの正規化
/// </summary>
/// /// <param name="vector">正規化するベクトル</param>
/// /// <returns>正規化されたベクトル</returns>
Vector3 Normalize(const Vector3& vector) {
	float length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	if (length > 0.0f) {
		return { vector.x / length, vector.y / length, vector.z / length };
	}
	return { 0.0f, 0.0f, 0.0f };
}

/// <summary>
/// ベクトルの加算
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>加算の結果</returns>
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

/// <summary>
/// ベクトルの減算
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>減算の結果</returns>
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

/// <summary>
/// ベクトルの内積を求める
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>内積の結果</returns>
Vector3 Multiply(const Vector3& v, float scalar) {
	Vector3 result;
	result.x = v.x * scalar;
	result.y = v.y * scalar;
	result.z = v.z * scalar;
	return result;
}

/// <summary>
/// 行列とスカラー値の掛け算
/// </summary>
/// <param name="x">行列</param>
/// <param name="scalar">スカラー値</param>
/// <returns>掛け算結果</returns>
Matrix4x4 Muiltiply(const Matrix4x4& x, float scalar) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = x.m[i][j] * scalar;
		}
	}
	return result;
}

/// <summary>
/// 2つのベクトルの角度を求める
/// </summary>	
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>ラジアン単位の角度</returns>
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f }; // XY平面上のベクトル
	}
	return { 0.0f, -vector.z, vector.y }; // Z軸に平行なベクトル
}


/// <summary>
/// ベクトルv1をv2に投影する
/// </summary>
/// <param name="v1">投影されるベクトル</param>
/// <param name="v2">投影先のベクトル</param>
/// <returns>v1をv2に投影した結果のベクトル</returns>
Vector3 Project(const Vector3& v1, const Vector3& v2)
{
	float length = std::sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	if (length == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	Vector3 unitV2 = { v2.x / length, v2.y / length, v2.z / length };
	float dotProduct = v1.x * unitV2.x + v1.y * unitV2.y + v1.z * unitV2.z;
	return { unitV2.x * dotProduct, unitV2.y * dotProduct, unitV2.z * dotProduct };
}

/// <summary>
/// セグメント上の点とセグメントの最近接点を求める
/// </summary>	
/// <param name="point">点</param>
/// <param name="segment">セグメント</param>
/// <returns>セグメント上の最近接点</returns>
Vector3 ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 segmentVector = segment.diff - segment.origin;
	Vector3 pointToOrigin = point - segment.origin;

	float segmentLengthSquared = segmentVector * segmentVector;
	if (segmentLengthSquared == 0.0f) {
		return segment.origin;
	}

	// 投影係数
	float t = (pointToOrigin * segmentVector) / segmentLengthSquared;
	t = std::clamp(t, 0.0f, 1.0f);

	return segment.origin + segmentVector * t;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;
/// <summary>
/// 行列をスクリーンに表示
/// </summary>
/// <param name="x">行数</param>
/// <param name="y">列数</param>
/// <param name="m">4x4行列</param>
void MatrixScreenPrintf(int x, int y, Matrix4x4& m, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth, y + (row + 1) * kRowHeight, "%6.02f", m.m[row][column]);
		}
	}
}


void DrawLine(const Segment& segment, const Matrix4x4& vpMatrix, const Matrix4x4& viewportMatrix, uint32_t color){
	Vector3 ndcStart = Transform(segment.origin, vpMatrix);
	Vector3 ndcEnd = Transform(segment.diff, vpMatrix);
	Vector3 screenStart = Transform(ndcStart, viewportMatrix);
	Vector3 screenEnd = Transform(ndcEnd, viewportMatrix);
	Novice::DrawLine(
		int(screenStart.x), int(screenStart.y),
		int(screenEnd.x), int(screenEnd.y),
		color
	);
}

/// <summary>
/// グリッドを描画
/// </summary>
/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
/// <param name="viewportMatrix">ビューポート変換行列</param>
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	// グリッドのサイズと分割数
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
	const float kGridZOffset = 0.0f;
	const uint32_t kBlack = 0x000000FF;
	
	// グリッドの描画
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex)
	{
		Vector3 start = { -kGridHalfWidth + kGridEvery * xIndex, 0.0f, -kGridHalfWidth + kGridZOffset };
		Vector3 end = { -kGridHalfWidth + kGridEvery * xIndex, 0.0f, kGridHalfWidth + kGridZOffset };
		Vector3 ndcStart = Transform(start, viewProjectionMatrix);
		Vector3 ndcEnd = Transform(end, viewProjectionMatrix);
		Vector3 screenStart = Transform(ndcStart, viewportMatrix);
		Vector3 screenEnd = Transform(ndcEnd, viewportMatrix);

		uint32_t color = (fabsf(start.x) < 0.0001f) ? kBlack : WHITE;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex)
	{
		Vector3 start = { -kGridHalfWidth, 0.0f, -kGridHalfWidth + kGridEvery * zIndex + kGridZOffset };
		Vector3 end = { kGridHalfWidth, 0.0f, -kGridHalfWidth + kGridEvery * zIndex + kGridZOffset };
		Vector3 ndcStart = Transform(start, viewProjectionMatrix);
		Vector3 ndcEnd = Transform(end, viewProjectionMatrix);
		Vector3 screenStart = Transform(ndcStart, viewportMatrix);
		Vector3 screenEnd = Transform(ndcEnd, viewportMatrix);

		uint32_t color = (fabsf(start.z - kGridZOffset) < 0.0001f) ? kBlack : WHITE;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

/// <summary>
/// 球を描画
/// </summary>
/// <param name="sphere">球の情報</param>
/// <param name="vpMatrix">ビュープロジェクション行列</param>
/// <param name="viewportMatrix">ビューポート変換行列</param>
/// <param name="color">描画色</param>
void DrawSphere(const Sphere& sphere, const Matrix4x4& vpMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 球の描画は緯度経度で分割して線を引く
	const uint32_t kSubdivision = 12;
	// 緯度経度の分割数
	const float kLonEvery = 2.0f * float(M_PI) / float(kSubdivision);
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	// 球の中心から半径分だけ離れた位置に点を配置
	for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
		float theta = kLonEvery * float(lon);
		for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
			float phiA = -float(M_PI) / 2.0f + kLatEvery * float(lat);
			float phiB = phiA + kLatEvery;

			Vector3 a = {
				sphere.center.x + sphere.radius * cosf(phiA) * cosf(theta),
				sphere.center.y + sphere.radius * sinf(phiA), 
				sphere.center.z + sphere.radius * cosf(phiA) * sinf(theta)
			};
			Vector3 b = {
				sphere.center.x + sphere.radius * cosf(phiB) * cosf(theta),
				sphere.center.y + sphere.radius * sinf(phiB),
				sphere.center.z + sphere.radius * cosf(phiB) * sinf(theta)
			};

			Vector3 ndcA = Transform(a, vpMatrix);
			Vector3 ndcB = Transform(b, vpMatrix);
			Vector3 screenA = Transform(ndcA, viewportMatrix);
			Vector3 screenB = Transform(ndcB, viewportMatrix);

			Novice::DrawLine(
				int(screenA.x), int(screenA.y),
				int(screenB.x), int(screenB.y),
				color
			);
		}
	}

	// 緯度ごとに線を引く
	for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
		float phi = -float(M_PI) / 2.0f + kLatEvery * float(lat);
		for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
			float thetaA = kLonEvery * float(lon);
			float thetaB = kLonEvery * float(lon + 1);

			Vector3 a = {
				sphere.center.x + sphere.radius * cosf(phi) * cosf(thetaA),
				sphere.center.y + sphere.radius * sinf(phi),
				sphere.center.z + sphere.radius * cosf(phi) * sinf(thetaA)
			};
			Vector3 b = {
				sphere.center.x + sphere.radius * cosf(phi) * cosf(thetaB),
				sphere.center.y + sphere.radius * sinf(phi),
				sphere.center.z + sphere.radius * cosf(phi) * sinf(thetaB)
			};

			Vector3 ndcA = Transform(a, vpMatrix);
			Vector3 ndcB = Transform(b, vpMatrix);
			Vector3 screenA = Transform(ndcA, viewportMatrix);
			Vector3 screenB = Transform(ndcB, viewportMatrix);

			Novice::DrawLine(
				int(screenA.x), int(screenA.y),
				int(screenB.x), int(screenB.y),
				color
			);
		}
	}
}

/// <summary>
/// 平面を描画
/// </summary>
/// <param name="plane">平面の情報</param>
/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
/// /// <param name="viewportMatrix">ビューポート変換行列</param>
/// <param name="color">描画色</param>
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 cneter = Multiply(plane.normal, plane.distance);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = { -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z };
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z };

	Vector3 points[4];
	for (uint32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(perpendiculars[index], 2.0f);
		Vector3 point = Add(cneter, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
}

/// <summary>
/// 球と球の衝突判定
/// </summary>
/// <param name="sphere1">球1</param>
/// <param name="sphere2">球2</param>
/// <returns>判定結果</returns>
bool isCollision(const Sphere& sphere1, const Sphere& sphere2) {
	// 球の中心間の距離を計算
	Vector3 diff = Subtract(sphere1.center, sphere2.center);
	// 距離の二乗を計算し、半径の和の二乗と比較
	float distanceSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	float radiusSum = sphere1.radius + sphere2.radius;
	return distanceSquared <= (radiusSum * radiusSum);
}

/// <summary>
/// 球と四角形の衝突判定
/// </summary>
/// <param name="sphere1">球</param>
/// <param name="sphere2">四角形</param>
/// <returns>判定結果</returns>
bool isCollision(const Sphere& sphere, const Plane& plane) {
	// 平面中心点
	Vector3 center = { plane.normal.x * plane.distance, plane.normal.y * plane.distance, plane.normal.z * plane.distance };

	// 平面上の2つの直交ベクトルを作成
	Vector3 u = Normalize(Perpendicular(plane.normal));
	Vector3 v = Normalize(Cross(plane.normal, u));

	// 平面の4隅を計算（2x2の正方形と仮定）
	float halfSize = 2.0f;
	Vector3 corners[4];
	corners[0] = Add(center, Add(Multiply(u, halfSize), Multiply(v, halfSize)));
	corners[1] = Add(center, Add(Multiply(u, halfSize), Multiply(v, -halfSize)));
	corners[2] = Add(center, Add(Multiply(u, -halfSize), Multiply(v, halfSize)));
	corners[3] = Add(center, Add(Multiply(u, -halfSize), Multiply(v, -halfSize)));

	// 平面上に球の中心を射影
	float d = plane.normal.x * (sphere.center.x - center.x) +
		plane.normal.y * (sphere.center.y - center.y) +
		plane.normal.z * (sphere.center.z - center.z);
	Vector3 projected = {
		sphere.center.x - plane.normal.x * d,
		sphere.center.y - plane.normal.y * d,
		sphere.center.z - plane.normal.z * d
	};

	// 射影点が平面の矩形内にあるか判定
	Vector3 rel = Subtract(projected, center);
	float uDot = rel.x * u.x + rel.y * u.y + rel.z * u.z;
	float vDot = rel.x * v.x + rel.y * v.y + rel.z * v.z;
	if (uDot < -halfSize || uDot > halfSize || vDot < -halfSize || vDot > halfSize) {
		// 射影点が矩形外なら、矩形の最近点を求める
		uDot = std::clamp(uDot, -halfSize, halfSize);
		vDot = std::clamp(vDot, -halfSize, halfSize);
		projected = Add(center, Add(Multiply(u, uDot), Multiply(v, vDot)));
	}

	// 球の中心と最近点の距離で判定
	Vector3 diff = Subtract(sphere.center, projected);
	float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	return distSq <= sphere.radius * sphere.radius;
}

/// <summary>
/// 球と平面の衝突判定
/// </summary>
/// <param name="sphere1">球</param>
/// <param name="plane">平面</param>	
/// <returns>判定結果</returns>
bool isCollisionBoundless(const Sphere& sphere1, const Plane&plane) {
	// 平面の法線ベクトルと球の中心点から平面までの距離を計算
	float distance = plane.normal.x * sphere1.center.x +
		plane.normal.y * sphere1.center.y +
		plane.normal.z * sphere1.center.z -
		plane.distance;
	// 球の半径と平面までの距離を比較
	return fabsf(distance) <= sphere1.radius;
}

/// <summary>
/// 線と平面の衝突判定
/// </summary>
/// <param name="segment">線分</param>
/// <param name="plane">平面</param>
/// <returns>>判定結果</returns>
bool isCollisionLine2Plane(const Segment& segment, const Plane& plane) {
	// 線分の始点と終点を平面に投影
	Vector3 startToPlane = Subtract(segment.origin, Multiply(plane.normal, plane.distance));
	Vector3 endToPlane = Subtract(segment.diff, Multiply(plane.normal, plane.distance));
	// 線分の始点と終点が平面の同じ側にあるかどうかを判定
	return (plane.normal.x * startToPlane.x + plane.normal.y * startToPlane.y + plane.normal.z * startToPlane.z) *
		(plane.normal.x * endToPlane.x + plane.normal.y * endToPlane.y + plane.normal.z * endToPlane.z) < 0.0f;
}



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int kWindowWidth = 1280;
	int kWindowHeight = 720;

	Vector3 rotate = { 0.0f,0.0f,0.0f };
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 translate = { 0.0f,0.0f,0.0f };

	Vector3 cameraPostion{ 0.0f,4.0f,-10.0f };
	Vector3 cameraRotate{ 0.3f,0.0f,0.0f };

	Plane plane = { { 0.0f, 1.0f, 0.0f }, 1.0f };
	Segment segment = { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };
	unsigned int SegmentColor = 0xFFFFFFFF;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraPostion);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Muiltiply(worldMatrix, Muiltiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (keys[DIK_W] != 0) {
			rotate.x += 0.01f; // X軸回転
		}
		if (keys[DIK_S] != 0) {
			rotate.x -= 0.01f; // X軸回転
		}
		if (keys[DIK_A] != 0) {
			rotate.y += 0.01f; // Y軸回転
		}
		if (keys[DIK_D] != 0) {
			rotate.y -= 0.01f; // Y軸回転
		}
		if (keys[DIK_Q] != 0) {
			rotate.z += 0.01f; // Z軸回転
		}
		if (keys[DIK_E] != 0) {
			rotate.z -= 0.01f; // Z軸回転
		}


#ifdef _DEBUG
        ImGui::Begin("Debug Window");
        ImGui::SetWindowSize(ImVec2(300, 200));
        ImGui::DragFloat3("Plane Normal", &plane.normal.x, 0.01f);
        ImGui::DragFloat("Plane Distance", &plane.distance, 0.01f);
        ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.01f);
        ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.01f);
        // セグメント全体の移動用
        static Vector3 segmentOffset = { 0.0f, 0.0f, 0.0f };
        if (ImGui::DragFloat3("Segment Offset", &segmentOffset.x, 0.01f)) {
        Vector3 offsetDelta = segmentOffset;
        segment.origin = Add(segment.origin, offsetDelta);
        segment.diff = Add(segment.diff, offsetDelta);
        // オフセットをリセット
        segmentOffset = { 0.0f, 0.0f, 0.0f };
        }
        ImGui::End();

#endif // _DEBUG

		// 平面の法線を正規化
		plane.normal = Normalize(plane.normal);

		if (isCollisionLine2Plane(segment, plane)) {
			SegmentColor = 0xFF0000FF;
		}
		else {
			SegmentColor = 0xFFFFFFFF;
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(worldViewProjectionMatrix, viewportMatrix);
		DrawPlane(plane, worldViewProjectionMatrix, viewportMatrix, WHITE);
		DrawLine(segment, worldViewProjectionMatrix, viewportMatrix, SegmentColor);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}