#include "Cherry.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	Cherry::Cherry() {

		//GameObject.hの値初期化
		pos = Vec2(0, 0);
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Cherry;
		canPlayerKill = true;
		isDelete = false;
		isOutOfScreen = false;

		speed = 0;
		dir = 0;
	}

	void Cherry::update() {
		checkOutOfScreen();

		if (speed != 0) {
			calculateSpeed();

			// 位置更新
			pos.x += hspeed;
			pos.y += vspeed;
		}
		// 当たり判定位置更新
		hitBox->setPos(pos);
	}

	void Cherry::draw() const {
		TextureAsset(U"sprCherry").drawAt(pos.x,pos.y-1);
		//hitBox->draw(Palette::Blue);//判定の可視化
	}

	//speedとdirからhspeedとvspeedを計算
	void Cherry::calculateSpeed() {
		//ラジアンに変換
		double rad = Math::ToRadians(dir);

		hspeed = speed * Math::Cos(rad);
		vspeed = -speed * Math::Sin(rad);
	}

	//画面外判定
	void Cherry::checkOutOfScreen() {
		const int32 excess = hitBoxSize * 2;//画面端からの余白
		if (pos.x < -1 * excess || pos.x > Global::windowWidth + excess ||
			pos.y < -1 * excess || pos.y > Global::windowHeight + excess) {
			isOutOfScreen = true;
		}
		else {
			isOutOfScreen = false;
		}
	}

	void Cherry::onCollision(GameObject& other) {
	}
}
