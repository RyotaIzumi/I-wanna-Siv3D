#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Spike : public GameObject {
	private:
		int32 side = 32;
		int32 direction;//針の向き
	public:
		Spike(Vec2 startPos, int32 dir);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;
	};
}
