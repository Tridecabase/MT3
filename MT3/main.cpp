#include <Novice.h>
#include "Vector3.h"
#include <math.h>

const char kWindowTitle[] = "GC1B_08_ジョ_シセイ";

/// <summary>
/// 三次元ベクトルの加算
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns>加算結果</returns>
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

/// <summary>
/// 三次元ベクトルの減算
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns>減算結果</returns>
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

/// <summary>
/// 三次元ベクトルのスカラー倍
/// </summary>
/// <param name="scalar"></param>
/// <param name="v"></param>
/// <returns>スカラー倍後のベクトル</returns>
Vector3 Multiply(float scalar, const Vector3& v) {
	return Vector3(scalar * v.x, scalar * v.y, scalar * v.z);
}

/// <summary>
/// 三次元ベクトルの内積
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns>内積計算の結果</returns>
float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/// <summary>
/// 三次元ベクトルの長さを計算する
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
float Length(const Vector3& v) {
	return sqrtf(Dot(v, v));
}

/// <summary>
/// 三次元ベクトルの正規化
/// </summary>
/// <param name="v"></param>
/// <returns>正規化後のベクトル</returns>
Vector3 Normalize(const Vector3& v) {
	float length = Length(v);
	if (length == 0.0f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return Multiply(1.0f / length, v);
}

static const int kColumWidth = 60;
static const int kRowHeight = 20;
// ベクトルを画面に表示する関数
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%0.2f", vector.x);
	Novice::ScreenPrintf(x + kColumWidth, y, "%0.2f", vector.y);
	Novice::ScreenPrintf(x + kColumWidth * 2, y, "%0.2f", vector.z);
	Novice::ScreenPrintf(x + kColumWidth * 3, y, "%s", label);
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

		Vector3 v1{ 1.0f,3.0f,-5.0f };
		Vector3 v2{ 4.0f,-1.0f,2.0f };
		float k = { 4.0f };

		Vector3 resultAdd = Add(v1, v2);
		Vector3 resultSubtract = Subtract(v1, v2);
		Vector3 resultMultiply = Multiply(k, v1);
		float resultDot = Dot(v1, v2);
		float resultLength = Length(v1);
		Vector3 resultNormalize = Normalize(v2);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, resultAdd, " : Add");
		VectorScreenPrintf(0, kRowHeight, resultSubtract, " : Subtract");
		VectorScreenPrintf(0, kRowHeight * 2, resultMultiply, "	: Multiply");
		Novice::ScreenPrintf(0, kRowHeight * 3, "%0.2f : Dot", resultDot);
		Novice::ScreenPrintf(0, kRowHeight * 4, "%0.2f : Length", resultLength);
		VectorScreenPrintf(0, kRowHeight * 5, resultNormalize, " : Normalize");

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
