#pragma once
#include <Siv3D.hpp>
#include "../Global.h"
#include "SpriteEnum.h"

namespace Iwanna {

	struct SpriteData {
		String spriteName;          // 使用sprite名
		int32 frameCount = 1;     // フレーム数
		double image_speed = 1.0;  // 全体の再生時間
		double totalDuration = 1.0; // 総再生時間
		bool loop = true;            // ループ再生するか
		bool haveLR = false;         //左右差分を持つか

		// 等間隔アニメーション用コンストラクタ
		SpriteData() = default;

		SpriteData(const String& name, int count, double speed, bool loop = true, bool lr = true) : spriteName(name), frameCount(count), image_speed(speed), loop(loop), haveLR(lr) {
			totalDuration = image_speed * frameCount;
		}
	};

	class SpriteSystem {
	private:
		HashTable<AnimationAction, SpriteData> animations; // 登録済みアニメーション
		AnimationAction currentAction = AnimationAction::PLAYER_WAIT; // 現在のアクション
		Stopwatch stopwatch; // 経過時間計測用
		int frameWidth = 64;
		int frameHeight = 64;

		mutable bool isFinishedAnimation = false;

	public:
		SpriteSystem() = default;
		SpriteSystem(int frameW, int frameH);

		void addAnimation(AnimationAction action, const SpriteData& data);
		void setAction(AnimationAction action);
		void changeWidthHeight(int frameW, int frameH) {
			frameWidth = frameW;
			frameHeight = frameH;
		}
		void reset();
		TextureRegion getTextureRegion(Global::Direction direction) const;

		bool getIsFinishedAnimation() const { return isFinishedAnimation; };//非ループ時、animation再生が終わったかどうかを取得
	};
}
