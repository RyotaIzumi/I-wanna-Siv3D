#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Cherry : public GameObject {
	public:
		using Behavior = std::function<void(Cherry&, int32)>;

	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 10;
		int32 age = 0;
		Behavior behavior;

	public:
		double speed = 0;
		double dir = 0;

		bool isDelete = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Cherry();

		void update() override;
		void draw() const override;

		void setBehavior(const Behavior& newBehavior);
		int32 getAge() const;

		void calculateSpeed();
		void checkOutOfScreen();

		void onCollision(GameObject& other) override;
	};
}
