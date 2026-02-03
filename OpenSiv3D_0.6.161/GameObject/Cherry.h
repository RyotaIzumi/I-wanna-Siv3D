#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Cherry : public GameObject {
	private:
		double hspeed;
		double vspeed;

		//当たり判定サイズ
		int32 hitBoxSize = 10;

		//アニメーション管理用変数
		SpriteSystem spriteSystem;

	public:
		Cherry();

		void update() override;
		void draw() const override;

		void onCollision(GameObject& other) override;
	};
}
