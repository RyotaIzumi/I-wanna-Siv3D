#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Cherry : public GameObject {
	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 10;

	public:
		double speed = 0;
		double dir = 0;

		bool isDelete = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Cherry();

		void update() override;
		void draw() const override;

		void calculateSpeed();
		void checkOutOfScreen();

		void onCollision(GameObject& other) override;
	};
}
