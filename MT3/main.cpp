#include <Novice.h>
#include "Vector3.h"
#include <cmath>
#include "Matrix4x4.h"

const char kWindowTitle[] = "GC2B_05_ジョ_シセイ";

/// <summary>
/// x軸回りの回転行列を作成
/// </summary>
/// <param name="radian">角度</param>
/// <returns>回転行列</returns>
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 matrix = {};
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = std::cos(radian);
	matrix.m[1][2] = std::sinf(radian);
	matrix.m[2][1] = -std::sinf(radian);
	matrix.m[2][2] = std::cosf(radian);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

/// <summary>
/// y軸回りの回転行列を作成
/// </summary>
/// <param name="radian">角度</param>
/// <returns>回転行列</returns>
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 matrix = {};
	matrix.m[0][0] = std::cos(radian);
	matrix.m[0][2] = -std::sinf(radian);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][0] = std::sinf(radian);
	matrix.m[2][2] = std::cosf(radian);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

/// <summary>
/// z軸回りの回転行列を作成
/// </summary>
/// <param name="radian">角度</param>
/// <returns>回転行列</returns>
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 matrix = {};
	matrix.m[0][0] = std::cos(radian);
	matrix.m[0][1] = std::sinf(radian);
	matrix.m[1][0] = -std::sinf(radian);
	matrix.m[1][1] = std::cosf(radian);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
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

		Vector3 rotate{ 0.4f,1.43f,-0.8f };
		Matrix4x4 rotateXmatrix = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateYmatrix = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZmatrix = MakeRotateZMatrix(rotate.z);
		Matrix4x4 rotateXYZMatrix = Muiltiply(rotateXmatrix, Muiltiply(rotateYmatrix, rotateZmatrix));

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, rotateXmatrix, "rotateXmatrix");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateYmatrix, "rotateYmatrix");
		MatrixScreenPrintf(0, kRowHeight * 10, rotateZmatrix, "rotateZmatrix");
		MatrixScreenPrintf(0, kRowHeight * 15, rotateXYZMatrix, "rotateXYZMatrix");

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
