#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Block : public GameObject {
	public:
		Block(Vec2 startPos);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;
	};
}
