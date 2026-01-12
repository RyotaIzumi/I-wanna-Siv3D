#pragma once
#include <Siv3D.hpp>

namespace Iwanna {

	struct SpriteData {
		String spriteName;          // 使用sprite名
		double image_speed = 1.0;  // 全体の再生時間
		bool loop = true;            // ループ再生するか
		bool haveLR = false;         //左右差分を持つか
		Array<size_t> frameOrders;   //表示順

		// 等間隔アニメーション用コンストラクタ
		SpriteData() = default;

		SpriteData(const String& name, int frameCount, double duration, bool loop = true, bool lr = true) : spriteName(name), totalDuration(duration), loop(loop), haveLR(lr) {
			double interval = duration / frameCount;
			for (int i = 0; i < frameCount; ++i) {
				frameTimes << interval;
			}
		}
	};

	class AnimationSystem {
	private:
		HashTable<AnimationAction, SpriteData> animations; // 登録済みアニメーション
		AnimationAction currentAction = AnimationAction::PLAYER_WAIT; // 現在のアクション
		Stopwatch stopwatch; // 経過時間計測用
		int frameWidth = 64;
		int frameHeight = 64;

		mutable bool isFinishedAnimation = false;

	public:
		AnimationSystem() = default;
		AnimationSystem(int frameW, int frameH);

		void addAnimation(AnimationAction action, const SpriteData& data);
		void setAction(AnimationAction action);
		void changeWidthHeight(int frameW, int frameH) {
			frameWidth = frameW;
			frameHeight = frameH;
		}
		void reset();
		TextureRegion getTextureRegion(Direction direction) const;
		TextureRegion getEffectTextureRegion(BossAnimationEffect effect, double elapsedTime, Direction direction) const;

		bool getIsFinishedAnimation() const { return isFinishedAnimation; };//非ループ時、animation再生が終わったかどうかを取得
	};
}
