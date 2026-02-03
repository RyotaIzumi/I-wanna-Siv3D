#include "Block.h"

namespace Iwanna {
	Block::Block(Vec2 startPos) {
		//GameObject.hの値初期化
		pos = startPos;
		hitBox = std::make_shared<RectHitBox>(pos, SizeF{ 32, 32 });
		type = ObjectType::Block;
	}
	void Block::update() {
	}
	void Block::draw() const {
		hitBox->draw(Palette::Gray);
	}
	void Block::onCollision(GameObject& other) {
	}

}
