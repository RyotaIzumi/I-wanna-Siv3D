#pragma once

namespace Global {
	// ======================
   // Enum 定義
   // ======================

	enum class Direction {
		LEFT,
		RIGHT
	};

	// ======================
	// 共有変数宣言
	// ======================

	//操作入力用変数
	inline Input inputLeft = KeyLeft;
	inline Input inputRight = KeyRight;
	inline Input inputJump = KeyShift;
	inline Input inputShoot = KeyZ;
	inline Input inputStart = KeyShift;
	inline Input inputRestart = KeyR;

	 // ======================
	// 定数
	// ======================
}
