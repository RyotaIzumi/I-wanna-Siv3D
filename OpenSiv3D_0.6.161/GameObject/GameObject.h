#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"

namespace Iwanna {

	enum class ObjectType {
		Player,
		Enemy,
		Block,
		Bullet
	};

	class GameObject {
	public:
		Vec2 pos;
		std::shared_ptr<HitBox> hitBox;
		ObjectType type;

		virtual ~GameObject() = default;

		virtual void update() = 0;
		virtual void draw() const = 0;

		bool intersects(const GameObject& other) const {
			return hitBox->intersects(*other.hitBox);
		}

		virtual void onCollision(GameObject& other) = 0;
	};
}
