#include "Trigger.h"

namespace Iwanna {
	Trigger::Trigger(Vec2 startPos, int32 id, double sizeX, double sizeY) {
		//GameObject.hの値初期化
		pos = { startPos.x * side, startPos.y * side };
		hitBox = std::make_shared<RectHitBox>(pos, SizeF{ side * sizeX,side * sizeY });
		type = ObjectType::Trigger;
		canPlayerKill = false;
		trapID = id;
	}
	void Trigger::update() {
	}
	void Trigger::draw() const {
		//hitBox->draw(Palette::Pink);
	}
	void Trigger::onCollision(GameObject& other) {
	}

	// トリガーを作動させる
	void Trigger::triggerActivate() {
		isActivated = true;
	}
	// トリガーが作動しているかどうかを取得
	bool Trigger::getIsActivated() const {
		return isActivated;
	}
	// 罠のIDを取得
	int32 Trigger::getTrapID() const {
		return trapID;
	}
}
