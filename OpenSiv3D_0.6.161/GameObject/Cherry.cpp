#include "Cherry.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	Cherry::Cherry() {

		//GameObject.hの値初期化
		pos = Vec2(0, 0);
		hitBox = std::make_shared<CircleHitBox>(pos, hitBoxSize);
		type = ObjectType::Cherry;
		canPlayerKill = true;
		canPlayerKillAtFullAlpha = true;
		canPlayerKillBeforeFullAlpha = false;
		manualCanPlayerKillControl = false;
		isDelete = false;
		isOutOfScreen = false;
		canDeleteOutOfScreen = true;
		depth = DrawDepth::Cherry;

		speed = 0;
		dir = 0;
		age = 0;
		textureName = U"sprCherry";
		color = Palette::White;
		scale = 1.0;
		appearanceEffect = CherryEffect::None;
	}

	void Cherry::reset(const Vec2& newPos, const Settings& settings) {
		pos = newPos;
		hspeed = 0.0;
		vspeed = 0.0;
		speed = 0.0;
		dir = 0.0;
		direction = 0.0;
		gravity = 0.0;
		textureAngle = 0.0;
		age = 0;
		appearanceElapsed = 0;
		appearanceEffect = CherryEffect::None;
		alpha = 1.0;
		canPlayerKillBeforeFullAlpha = false;
		manualCanPlayerKillControl = false;
		isDelete = false;
		GameObject::isDelete = false;
		isOutOfScreen = false;
		hitBox->setPos(pos);
		applySettings(settings);
	}

	void Cherry::deactivate() {
		// Behavior が保持する controller / state を、プール待機中は解放する。
		behavior = nullptr;
		isDelete = false;
		GameObject::isDelete = false;
		isOutOfScreen = false;
		canPlayerKill = false;
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
		updateAppearanceEffect();
		if (!manualCanPlayerKillControl) {
			canPlayerKill = canPlayerKillAtFullAlpha && (canPlayerKillBeforeFullAlpha || alpha >= 1.0);
		}
		// 当たり判定位置更新
		hitBox->setPos(pos);
		if(canDeleteOutOfScreen) checkOutOfScreen();
		++age;
	}

	void Cherry::draw() const {
		ColorF drawColor = color;
		drawColor.a *= alpha;
		TextureAsset(textureName).scaled(scale).drawAt(pos.x,pos.y-1, drawColor);
		//hitBox->draw(Palette::Blue);//判定の可視化
	}

	void Cherry::applySettings(const Settings& settings) {
		textureName = settings.textureName;
		color = settings.color;
		behavior = settings.behavior;
		canDeleteOutOfScreen = settings.canDeleteOutOfScreen;
		canPlayerKillAtFullAlpha = settings.canPlayerKill;
		canPlayerKillBeforeFullAlpha = settings.canPlayerKillBeforeFullAlpha;
		manualCanPlayerKillControl = settings.manualCanPlayerKillControl;
		depth = settings.depth;
		setScale(settings.scale);
		alpha = Clamp(settings.alpha, 0.0, 1.0);
		setAppearanceEffect(settings.appearanceEffect, settings.appearanceDuration);
		if (!manualCanPlayerKillControl) {
			canPlayerKill = canPlayerKillAtFullAlpha && (canPlayerKillBeforeFullAlpha || alpha >= 1.0);
		}
		else {
			canPlayerKill = settings.canPlayerKill;
		}
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

	void Cherry::setScale(double newScale) {
		scale = Max(newScale, 0.0);
		auto& circleHitBox = static_cast<CircleHitBox&>(*hitBox);
		circleHitBox.circle.r = hitBoxSize * scale;
	}

	void Cherry::setEffect(CherryEffect effect, int32 duration) {
		appearanceEffect = effect;
		appearanceElapsed = 0;
		appearanceDuration = Max(duration, 1);
		appearanceTargetScale = scale;
		appearanceTargetAlpha = alpha;

		switch (appearanceEffect) {
		case CherryEffect::FadeIn:
			alpha = 0.0;
			break;
		case CherryEffect::ScaleIn:
			setScale(0.0);
			break;
		case CherryEffect::FadeOut:
		case CherryEffect::ScaleOut:
		case CherryEffect::None:
		default:
			break;
		}
	}

	void Cherry::setAppearanceEffect(CherryEffect effect, int32 duration) {
		setEffect(effect, duration);
	}

	void Cherry::updateAppearanceEffect() {
		if (appearanceEffect == CherryEffect::None) {
			return;
		}

		const double t = Clamp(
			appearanceElapsed / static_cast<double>(appearanceDuration),
			0.0,
			1.0);
		const double eased = t * t * (3.0 - 2.0 * t);

		switch (appearanceEffect) {
		case CherryEffect::FadeIn:
			alpha = appearanceTargetAlpha * eased;
			break;
		case CherryEffect::ScaleIn:
			setScale(appearanceTargetScale * eased);
			break;
		case CherryEffect::FadeOut:
			alpha = appearanceTargetAlpha * (1.0 - eased);
			break;
		case CherryEffect::ScaleOut:
			setScale(appearanceTargetScale * (1.0 - eased));
			break;
		case CherryEffect::None:
		default:
			break;
		}

		++appearanceElapsed;
		if (appearanceElapsed > appearanceDuration) {
			const bool shouldDelete = (appearanceEffect == CherryEffect::FadeOut)
				|| (appearanceEffect == CherryEffect::ScaleOut);

			if (appearanceEffect == CherryEffect::FadeIn) {
				alpha = appearanceTargetAlpha;
			}
			else if (appearanceEffect == CherryEffect::ScaleIn) {
				setScale(appearanceTargetScale);
			}
			else if (appearanceEffect == CherryEffect::FadeOut) {
				alpha = 0.0;
			}
			else if (appearanceEffect == CherryEffect::ScaleOut) {
				setScale(0.0);
			}

			appearanceEffect = CherryEffect::None;
			if (shouldDelete) {
				isDelete = true;
			}
		}
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
		const double excess = hitBoxSize * scale * 2;//画面端からの余白
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
