#pragma once
#include <Siv3D.hpp>

namespace Iwanna {
	class ScreenEffect {
	private:
		enum class State {
			None,
			Fade,
			FlashHold,
			FlashFade,
		};

		State state = State::None;
		ColorF color = Palette::Black;
		double alpha = 0.0;
		double startAlpha = 0.0;
		double targetAlpha = 0.0;
		int32 elapsed = 0;
		int32 duration = 1;
		int32 flashHoldDuration = 1;
		int32 flashFadeDuration = 1;

		static double easeInOut(double t) {
			t = Clamp(t, 0.0, 1.0);
			return t * t * (3.0 - 2.0 * t);
		}

		void beginFade(double from, double to, int32 steps) {
			startAlpha = from;
			targetAlpha = to;
			alpha = from;
			elapsed = 0;
			duration = Max(steps, 1);
			state = State::Fade;
		}

	public:
		void reset() {
			state = State::None;
			alpha = 0.0;
		}

		// 現在の画面から指定色へ覆う。黒なら暗転になる。
		void fadeIn(int32 steps, const ColorF& fadeColor = Palette::Black) {
			color = fadeColor;
			beginFade(alpha, 1.0, steps);
		}

		// 指定色で覆われた状態からゲーム画面へ戻す。
		void fadeOut(int32 steps, const ColorF& fadeColor = Palette::Black) {
			color = fadeColor;
			beginFade((alpha > 0.0 ? alpha : 1.0), 0.0, steps);
		}

		// 指定色を即座に表示し、holdSteps 後に徐々に消す。
		void flash(int32 holdSteps, int32 fadeOutSteps, const ColorF& flashColor = Palette::White) {
			color = flashColor;
			alpha = 1.0;
			elapsed = 0;
			flashHoldDuration = Max(holdSteps, 0);
			flashFadeDuration = Max(fadeOutSteps, 1);
			state = (flashHoldDuration == 0 ? State::FlashFade : State::FlashHold);
		}

		void update() {
			switch (state) {
			case State::Fade: {
				++elapsed;
				const double t = easeInOut(static_cast<double>(elapsed) / duration);
				alpha = Math::Lerp(startAlpha, targetAlpha, t);
				if (elapsed >= duration) {
					alpha = targetAlpha;
					state = State::None;
				}
				break;
			}
			case State::FlashHold:
				if (++elapsed >= flashHoldDuration) {
					elapsed = 0;
					state = State::FlashFade;
				}
				break;
			case State::FlashFade: {
				++elapsed;
				const double t = easeInOut(static_cast<double>(elapsed) / flashFadeDuration);
				alpha = 1.0 - t;
				if (elapsed >= flashFadeDuration) {
					alpha = 0.0;
					state = State::None;
				}
				break;
			}
			default:
				break;
			}
		}

		void draw() const {
			if (alpha <= 0.0) return;
			Scene::Rect().draw(ColorF{ color.r, color.g, color.b, color.a * alpha });
		}
	};
}
