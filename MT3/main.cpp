#define _USE_MATH_DEFINES
#include <Novice.h>
#include "Vector3.h"
#include <cmath>
#include "Matrix4x4.h"
#include "imgui.h"

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

// クロス積
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

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 5.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
	const float kGridZOffset = 10.0f;
	const uint32_t kBlack = 0x000000FF;

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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int kWindowWidth = 1280;
	int kWindowHeight = 720;

	Vector3 cameraTranslate = { 0.0f, 3.6f, -6.49f };
	Vector3 cameraRotation = { 0.26f, 0.0f, 0.0f };

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

#ifdef _DEBUG
		ImGui::Begin("Debug Window");
		ImGui::SetWindowSize(ImVec2(300, 200));
		ImGui::Text("Camera Control");
		//カメラの回転、平行移動をスライダーで調整
		ImGui::SliderFloat3("Camera Rotation", &cameraRotation.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Camera Translate", &cameraTranslate.x, -10.0f, 10.0f);
		ImGui::End();
#endif // _DEBUG
		
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f, 1.0f ,1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotation, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Muiltiply(worldMatrix, Muiltiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(worldViewProjectionMatrix, viewportMatrix);

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