#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Block : public GameObject {
	private:
		enum class Role { Normal, Disposable, Goal };
		int32 side = 32;
		String textureName = U"sprBlock";
		bool hasCollide = true;
		Role role = Role::Normal;
		bool visited = false;
		int32 collapseFrames = 0;
	public:
		Block(String name, Vec2 startPos);

		void update() override;
		void draw() const override;
		void onCollision(GameObject& other) override;

		void setHasCollide(bool b);
		bool getHasCollide() const;
		void makeDisposable();
		void makeGoal();
		void activateDisposable();
		bool isDisposable() const;
		bool isVisited() const;
		bool isGoal() const;
	};
}
