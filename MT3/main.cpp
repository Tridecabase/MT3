#include <Novice.h>
#include "Vector3.h"
#include <cmath>
#include "Matrix4x4.h"

const char kWindowTitle[] = "GC2B_05_ジョ_シセイ";

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
/// <param name="scale"三次元ベクトル></param>
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

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

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

        Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
           Matrix4x4 result;

           // 回転角をラジアンに変換
           float cosX = cosf(rotate.x);
           float sinX = sinf(rotate.x);
           float cosY = cosf(rotate.y);
           float sinY = sinf(rotate.y);
           float cosZ = cosf(rotate.z);
           float sinZ = sinf(rotate.z);

           // スケール、回転、平行移動を組み合わせたアフィン変換行列を作成
           result.m[0][0] = scale.x * (cosY * cosZ);
           result.m[0][1] = scale.x * (cosY * sinZ);
           result.m[0][2] = scale.x * (-sinY);
           result.m[0][3] = 0.0f;

           result.m[1][0] = scale.y * (sinX * sinY * cosZ - cosX * sinZ);
           result.m[1][1] = scale.y * (sinX * sinY * sinZ + cosX * cosZ);
           result.m[1][2] = scale.y * (sinX * cosY);
           result.m[1][3] = 0.0f;

           result.m[2][0] = scale.z * (cosX * sinY * cosZ + sinX * sinZ);
           result.m[2][1] = scale.z * (cosX * sinY * sinZ - sinX * cosZ);
           result.m[2][2] = scale.z * (cosX * cosY);
           result.m[2][3] = 0.0f;

           result.m[3][0] = translate.x;
           result.m[3][1] = translate.y;
           result.m[3][2] = translate.z;
           result.m[3][3] = 1.0f;

           return result;
        }
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///


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
