#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Trigger : public GameObject {
	private:
		int32 side = 32;
		int32 trapID = 0;
		bool isActivated = false;//トリガーが作動しているかどうか
	public:
		Trigger(Vec2 startPos, int32 id, double sizeX, double sizeY);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;

		// トリガーを作動させる
		void triggerActivate();
		bool getIsActivated() const;
		int32 getTrapID() const;
	};
}
