#include <Siv3D.hpp>
#include "SpriteSystem.h"

namespace Iwanna {
	SpriteSystem::SpriteSystem(int frameW, int frameH) : frameWidth(frameW), frameHeight(frameH) {
		stopwatch.start();
	}

	void SpriteSystem::addAnimation(AnimationAction action, const SpriteData& data) {
		animations[action] = data;
	}

	void SpriteSystem::setAction(AnimationAction action) {
		if (currentAction != action) {
			currentAction = action;
			stopwatch.restart();
		}
	}

	AnimationAction SpriteSystem::getCurrentAction() const {
		return currentAction;
	}

	void SpriteSystem::reset() {
		stopwatch.restart();
	}

	// 現在のアニメーションフレームから TextureRegion を返す
	TextureRegion SpriteSystem::getTextureRegion(Global::Direction direction) const {
		if (!animations.contains(currentAction)) {
			return TextureRegion{}; // 登録がない場合は空
		}

		const auto& data = animations.at(currentAction);
		double t = stopwatch.sF();

		// 経過時間をループまたは上限で制限
		double localTime = t;
		if (data.loop) {
			localTime = Fmod(t, data.totalDuration);
		}
		else {
			localTime = Min(t, data.totalDuration - 0.001);
			isFinishedAnimation = (localTime >= data.totalDuration - 0.001);
		}

		// 現在のフレームを決定
		int frameIndex = 0;
		double accumulatedTime = 0.0;
		for (int i = 0; i < data.frameCount; ++i) {
			accumulatedTime += data.image_speed;
			if (localTime < accumulatedTime) {
				frameIndex = i;
				break;
			}
		}


		int texRange = frameIndex * frameWidth;

		String AddedTextureName = U"";
		if (data.haveLR) {
			AddedTextureName += data.spriteName;
			AddedTextureName += (direction == Global::Direction::RIGHT) ? U"_right" : U"_left";
		}

		// テクスチャ切り出しと左右反転
		TextureRegion texture = TextureAsset((data.haveLR) ? AddedTextureName : data.spriteName)
			(texRange, 0, frameWidth, frameHeight).mirrored(direction == Global::Direction::LEFT);

		return texture;
	}
}
