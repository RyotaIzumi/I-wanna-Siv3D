#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Miku : public GameObject {
	public:
		Miku(Vec2 startPos);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;
	};
}
