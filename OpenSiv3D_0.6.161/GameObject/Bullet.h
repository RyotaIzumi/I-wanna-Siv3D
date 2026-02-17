#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Bullet : public GameObject {
	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 2;

	public:
		bool isDelete = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Bullet(Vec2& pos,double speed);

		void update() override;
		void draw() const override;

		void onCollision(GameObject& other) override;
		void checkOutOfScreen();
	};
}
