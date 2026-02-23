#include "Spike.h"

namespace Iwanna {
	Spike::Spike(Vec2 startPos, int32 dir) {
		//GameObject.hの値初期化
		pos.x = startPos.x * side;
		pos.y = startPos.y * side;
		spriteDirection = dir;
		hitBox = std::make_shared<SpikeHitBox>(pos, dir);
		type = ObjectType::Spike;
		canPlayerKill = true;
	}

	void Spike::update() {
	}

	void Spike::trapUpdate(int32 id) {
	}

	void Spike::draw() const {
		switch (spriteDirection) {
		case 0:TextureAsset(U"sprSpikeUp").draw(pos); break;
		case 1:TextureAsset(U"sprSpikeLeft").draw(pos); break;
		case 2:TextureAsset(U"sprSpikeDown").draw(pos); break;
		case 3:TextureAsset(U"sprSpikeRight").draw(pos); break;
		}
		
		//hitBox->draw(ColorF(Palette::Blue,0.7));
	}

	void Spike::onCollision(GameObject& other) {
	}

	// ----- 以下罠用針 ----- //

	SpikeTrap::SpikeTrap(Vec2 startPos, int32 dir, int32 id, double direction, double speed) : Spike({ startPos.x, startPos.y }, dir), trapID(id), direction(direction), speed(speed) {
		hspeed = 0;
		vspeed = 0;
	}

	void SpikeTrap::trapUpdate(int32 id) {
		if (trapID == id) {
			calculateSpeed();
			pos.x += hspeed;
			pos.y += vspeed;
			hitBox->setPos(pos);
		}
	}

	//speedとdirからhspeedとvspeedを計算
	void SpikeTrap::calculateSpeed() {
		//ラジアンに変換
		double rad = Math::ToRadians(direction);

		hspeed = speed * Math::Cos(rad);
		vspeed = -speed * Math::Sin(rad);
	}

	//罠のIDを取得
	int32 SpikeTrap::getTrapID() const {
		return trapID;
	}
}
