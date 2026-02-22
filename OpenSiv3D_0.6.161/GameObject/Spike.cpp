#include "Spike.h"

namespace Iwanna {
	Spike::Spike(Vec2 startPos, int32 dir) {
		//GameObject.hの値初期化
		pos.x = startPos.x * side;
		pos.y = startPos.y * side;
		direction = dir;
		hitBox = std::make_shared<SpikeHitBox>(pos, dir);
		type = ObjectType::Spike;
		canPlayerKill = true;
	}
	void Spike::update() {
	}
	void Spike::draw() const {
		switch (direction) {
		case 0:TextureAsset(U"sprSpikeUp").draw(pos); break;
		case 1:TextureAsset(U"sprSpikeLeft").draw(pos); break;
		case 2:TextureAsset(U"sprSpikeDown").draw(pos); break;
		case 3:TextureAsset(U"sprSpikeRight").draw(pos); break;
		}
		
		hitBox->draw(ColorF(Palette::Blue,0.7));
	}
	void Spike::onCollision(GameObject& other) {
	}
}
