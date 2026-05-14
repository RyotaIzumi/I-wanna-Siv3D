#include "Blood.h"
#include "../Audio/AudioAsset.h"
#include "../GameObject/Block.h"

namespace Iwanna {
	Blood::Blood(Vec2& genePos, double dir) {

		//GameObject.hの値初期化
		pos = genePos;
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Blood;
		canPlayerKill = false;
		isStop = false;
		isOutOfScreen = false;

		gravity = 0.4;
		speed = Random(5, 12);

		depth = 41;

		setDirection(dir);
	}

	void Blood::update() {
		checkOutOfScreen();

		if (isStop)return;

		vspeed += gravity;
		// 位置更新
		pos.x += hspeed;
		pos.y += vspeed;

		// 当たり判定位置更新
		hitBox->setPos(pos);
	}

	void Blood::draw() const {
		//Rect(0,0,1,1).drawAt(pos.x, pos.y);
		hitBox->draw(Palette::Red);//判定の可視化(そのまま血に利用)
	}

	void Blood::onCollision(GameObject& other) {
		// ブロック衝突
		if (this->intersects(other) && other.type == ObjectType::Block && !isStop) {
			auto* block = dynamic_cast<Block*>(&other);
			if (block->getHasCollide())isStop = true;
		}
	}

	//画面外判定
	void Blood::checkOutOfScreen() {
		const int32 excess = hitBoxSize * 2;//画面端からの余白
		if (pos.x < -1 * excess || pos.x > Global::stageWidth + excess ||
			pos.y < -1 * excess || pos.y > Global::stageHeight + excess) {
			isOutOfScreen = true;
		}
		else {
			isOutOfScreen = false;
		}
	}

	//speedとdirからhspeedとvspeedを計算
	void Blood::calculateSpeed() {
		//ラジアンに変換
		double rad = Math::ToRadians(direction);

		hspeed = speed * Math::Cos(rad);
		vspeed = -speed * Math::Sin(rad);
	}

	//飛んでいく方角を設定
	void Blood::setDirection(double dir) {
		direction = dir;
		calculateSpeed();
	}
}
