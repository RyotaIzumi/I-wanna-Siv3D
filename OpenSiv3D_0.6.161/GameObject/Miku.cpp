#include "Miku.h"

namespace Iwanna {
	Miku::Miku(Vec2 startPos) {
		//GameObject.hの値初期化
		pos = startPos;
		hitBox = std::make_shared<MikuHitBox>(pos);
		type = ObjectType::Miku;
		canPlayerKill = true;
	}
	void Miku::update() {
	}
	void Miku::draw() const {
		TextureAsset(U"sprMiku").drawAt(pos.x - 7,pos.y - 4);
		//hitBox->draw(ColorF(Palette::White,0.7));
	}
	void Miku::onCollision(GameObject& other) {
	}
}
