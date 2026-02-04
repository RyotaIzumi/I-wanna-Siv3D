#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Block : public GameObject {
	private:
		int32 side = 32;
		String textureName = U"sprBlock";
	public:
		Block(String name, Vec2 startPos);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;
	};
}
