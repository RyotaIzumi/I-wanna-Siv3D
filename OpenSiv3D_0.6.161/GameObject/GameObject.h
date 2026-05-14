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
		SpecialTrap,
		SavePoint,
		Blood,
		Miku,
		Warp,
		Sign,
		Item
	};

	class GameObject {
	private:
		double prevSpd = 0.0;
		double prevDir = 0.0;
	public:
		Vec2 pos;
		std::shared_ptr<HitBox> hitBox;
		ObjectType type;
		bool canPlayerKill = false;
		bool isDelete = false; //消去用フラグ

		//基礎パラメータ
		double hspeed;
		double vspeed;
		double speed;
		double direction; // 移動方向
		double gravity;
		double alpha = 1.0;
		double depth = 0.0;

		double textureAngle = 0.0; // 描画の角度

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

		//speedとdirectionからhspeedとvspeedを計算
		void calculateSpeed() {
			//ラジアンに変換
			double rad = Math::ToRadians(direction);

			hspeed = speed * Math::Cos(rad);
			vspeed = -speed * Math::Sin(rad);
		}

		//speedかdirectionの値が変化したかどうかを取得
		bool isChangedDirOrSpd() {
			bool changed = (speed != prevSpd) || (direction != prevDir);

			// 更新
			prevSpd = speed;
			prevDir = direction;

			return changed;
		}

		//2つの座標から角度を計算
		void calculateDirection(Vec2 basePos, Vec2 targetPos) {
			Vec2 diff = targetPos - basePos;
			direction = Math::ToDegrees(Atan2(-diff.y, diff.x));
		}

		//2つの座標から距離を計算
		double calculateDistance(Vec2 basePos, Vec2 targetPos) {
			return basePos.distanceFrom(targetPos);
		}

		virtual void onCollision(GameObject& other) = 0;
	};

}
