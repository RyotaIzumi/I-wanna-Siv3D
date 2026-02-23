#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"

namespace Iwanna {

	enum class ObjectType {
		Player,
		Cherry,
		Block,
		Bullet,
		Spike,
		Trigger,
		Miku
	};

	class GameObject {
	public:
		Vec2 pos;
		std::shared_ptr<HitBox> hitBox;
		ObjectType type;
		bool canPlayerKill = false;

		double hspeed;
		double vspeed;

		virtual ~GameObject() = default;

		virtual void update() = 0;
		virtual void draw() const = 0;

		RectF getBroadRect() const {
			if (auto r = hitBox->getRect()) return *r;
			if (auto c = hitBox->getCircle()) return c->boundingRect();
			return RectF{};
		}

		bool intersects(const GameObject& other) const {
			return hitBox->intersects(*other.hitBox);
		}

		virtual void onCollision(GameObject& other) = 0;
	};
}
