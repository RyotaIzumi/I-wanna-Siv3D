#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Spike : public GameObject {
	private:
	protected:
		int32 side = 32;
		int32 spriteDirection;//針の向き
	public:
		Spike(Vec2 startPos, int32 dir);

		void update() override;
		virtual void trapUpdate(int32 id);
		void draw() const override;
		void onCollision(GameObject& other) override;
	};

	class SpikeTrap : public Spike {
	private:
		int32 trapID = 0;
		double direction = 0;
		double speed = 0;
	public:
		SpikeTrap(Vec2 startPos, int32 dir, int32 id, double direction, double speed);
		void trapUpdate(int32 id) override;
		void calculateSpeed();

		int32 getTrapID() const;
	};
}
