#include "Bullet.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	Bullet::Bullet(Vec2& genePos, double hs){

		//GameObject.hの値初期化
		pos = genePos;
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Bullet;
		canPlayerKill = false;
		isDelete = false;
		isOutOfScreen = false;

		hspeed = hs;
		vspeed = 0;
	}

	void Bullet::update() {
		checkOutOfScreen();
		// 位置更新
		pos.x += hspeed;
		pos.y += vspeed;
		
		// 当たり判定位置更新
		hitBox->setPos(pos);
	}

	void Bullet::draw() const {
		TextureAsset(U"sprBullet").drawAt(pos.x, pos.y);
		//hitBox->draw(Palette::Blue);//判定の可視化
	}

	void Bullet::onCollision(GameObject& other) {
		// ブロック、ミク衝突
		if (other.type == ObjectType::Block) {
			isDelete = true;
		}

		// ミク衝突
		if (this->intersects(other) && other.type == ObjectType::Miku) {
			isDelete = true;
		}
	}

	//画面外判定
	void Bullet::checkOutOfScreen() {
		const int32 excess = hitBoxSize * 2;//画面端からの余白
		if (pos.x < -1 * excess || pos.x > Global::windowWidth + excess ||
			pos.y < -1 * excess || pos.y > Global::windowHeight + excess) {
			isOutOfScreen = true;
		}
		else {
			isOutOfScreen = false;
		}
	}
}
