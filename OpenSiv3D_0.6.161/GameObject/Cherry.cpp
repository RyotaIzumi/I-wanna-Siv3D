#include "Cherry.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	Cherry::Cherry() {

		//GameObject.hの値初期化
		pos = Vec2(200, 400);
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Cherry;

		hspeed = 0.0;
		vspeed = 0.0;
	}

	void Cherry::update() {
	}

	void Cherry::draw() const {
		TextureAsset(U"sprCherry").drawAt(pos.x,pos.y-1);
		//hitBox->draw(Palette::Blue);//判定の可視化
	}

	void Cherry::onCollision(GameObject& other) {
	}
}
