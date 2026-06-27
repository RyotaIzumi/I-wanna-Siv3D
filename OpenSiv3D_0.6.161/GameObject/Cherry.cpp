#include "Cherry.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	Cherry::Cherry() {

		//GameObject.hの値初期化
		pos = Vec2(0, 0);
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Cherry;
		canPlayerKill = true;
		isDelete = false;
		isOutOfScreen = false;
		canDeleteOutOfScreen = true;
		depth = DrawDepth::Cherry;

		speed = 0;
		dir = 0;
		age = 0;
		textureName = U"sprCherry";
		color = Palette::White;
	}

	void Cherry::update() {
		if (behavior) {
			behavior(*this, age);
		}

		if (speed != 0) {
			calculateSpeed();

			// 位置更新
			pos.x += hspeed;
			pos.y += vspeed;
		}
		// 当たり判定位置更新
		hitBox->setPos(pos);
		if(canDeleteOutOfScreen) checkOutOfScreen();
		++age;
	}

	void Cherry::draw() const {
		const ScopedRenderStates2D rs{ SamplerState::ClampNearest };
		ColorF drawColor = color;
		drawColor.a *= alpha;
		TextureAsset(textureName).drawAt(pos.x,pos.y-1, drawColor);
		//hitBox->draw(Palette::Blue);//判定の可視化
	}

	void Cherry::applySettings(const Settings& settings) {
		textureName = settings.textureName;
		color = settings.color;
		behavior = settings.behavior;
		canDeleteOutOfScreen = settings.canDeleteOutOfScreen;
		depth = settings.depth;
	}

	void Cherry::setBehavior(const Behavior& newBehavior) {
		behavior = newBehavior;
	}

	void Cherry::setVisual(const String& newTextureName, const ColorF& newColor) {
		textureName = newTextureName;
		color = newColor;
	}

	void Cherry::setTextureName(const String& newTextureName) {
		textureName = newTextureName;
	}

	void Cherry::setColor(const ColorF& newColor) {
		color = newColor;
	}

	void Cherry::setCanDeleteOutOfScreen(bool enabled) {
		canDeleteOutOfScreen = enabled;
	}

	int32 Cherry::getAge() const {
		return age;
	}

	//speedとdirからhspeedとvspeedを計算
	void Cherry::calculateSpeed() {
		//ラジアンに変換
		double rad = Math::ToRadians(dir);

		hspeed = speed * Math::Cos(rad);
		vspeed = -speed * Math::Sin(rad);
	}

	//画面外判定
	void Cherry::checkOutOfScreen() {
		const int32 excess = hitBoxSize * 2;//画面端からの余白
		if (pos.x < -1 * excess || pos.x > Global::windowWidth + excess ||
			pos.y < -1 * excess || pos.y > Global::windowHeight + excess) {
			isOutOfScreen = true;
		}
		else {
			isOutOfScreen = false;
		}
	}

	void Cherry::onCollision(GameObject& other) {
	}
}
