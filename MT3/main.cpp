#include <Novice.h>
#include "Vector3.h"
#include <cmath>
#include "Matrix4x4.h"

const char kWindowTitle[] = "GC2B_05_ジョ_シセイ";

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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

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

		Matrix4x4 orthographicMatrix = MakeOrthographicMatrix(-160.0f, 160.0f, 200.0f, 300.0f, 0.0f, 1000.0f);
		Matrix4x4 perspectiveMatrix = MakePerspectiveFovMatrix(0.63f, 1.33f, 0.1f, 1000.0f);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(100.0f, 200.0f, 600.0f, 300.0f, 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, orthographicMatrix, "Orthographic Matrix");
		MatrixScreenPrintf(0, 5 * kRowHeight, perspectiveMatrix, "Perspective Matrix");
		MatrixScreenPrintf(0, 10 * kRowHeight, viewportMatrix, "Viewport Matrix");

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
