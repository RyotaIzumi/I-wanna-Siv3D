#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Blood : public GameObject {
	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 2;
		double speed = 10;
		double direction = 0;

	public:
		bool isStop = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Blood(Vec2& pos, double dir);

		void update() override;
		void draw() const override;

		void onCollision(GameObject& other) override;
		void checkOutOfScreen();
		void calculateSpeed();
		void setDirection(double dir);
	};
}
