#include "Block.h"

namespace Iwanna {
	Block::Block(String name, Vec2 startPos) {
		textureName = name;
		//GameObject.hの値初期化
		pos = {startPos.x * side, startPos.y * side};
		hitBox = std::make_shared<RectHitBox>(pos, SizeF{ side,side });
		type = ObjectType::Block;
		canPlayerKill = false;
	}
	void Block::update() {
	}
	void Block::draw() const {
		//hitBox->draw(Palette::Gray);
		TextureAsset(textureName).draw(pos);
	}
	void Block::onCollision(GameObject& other) {
	}

}
