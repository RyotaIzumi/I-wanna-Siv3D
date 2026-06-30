#pragma once
#include <Siv3D.hpp>

namespace Iwanna {

	enum class EasingMoveType {
		Linear = 0,
		EaseIn = 1,
		EaseOut = 2,
		EaseInOut = 3,
	};

	inline double applyEasing(EasingMoveType easingType, double t) {
		t = Clamp(t, 0.0, 1.0);

		switch (easingType) {
		case EasingMoveType::EaseIn:
			return t * t;
		case EasingMoveType::EaseOut:
			return 1.0 - (1.0 - t) * (1.0 - t);
		case EasingMoveType::EaseInOut:
			return t * t * (3.0 - 2.0 * t);
		case EasingMoveType::Linear:
		default:
			return t;
		}
	}

	class EasingValue {
	private:
		double startValue = 0.0;
		double targetValue = 0.0;
		int32 duration = 1;
		int32 elapsed = 0;
		EasingMoveType type = EasingMoveType::Linear;
		bool active = false;

	public:
		void start(EasingMoveType easingType, double currentValue, double newTargetValue, int32 stepCount) {
			startValue = currentValue;
			targetValue = newTargetValue;
			duration = Max(stepCount, 1);
			elapsed = 0;
			type = easingType;
			active = true;
		}

		double update() {
			if (!active) {
				return targetValue;
			}

			const double t = elapsed / static_cast<double>(duration);
			const double result = startValue + (targetValue - startValue) * applyEasing(type, t);

			++elapsed;
			if (elapsed > duration) {
				active = false;
				return targetValue;
			}

			return result;
		}

		bool isActive() const {
			return active;
		}
	};

	class EasingMove {
	private:
		Vec2 startPos = Vec2{ 0,0 };
		Vec2 targetPos = Vec2{ 0,0 };
		int32 duration = 1;
		int32 elapsed = 0;
		EasingMoveType type = EasingMoveType::Linear;
		bool active = false;

	public:
		void start(EasingMoveType easingType, const Vec2& currentPos, const Vec2& newTargetPos, int32 stepCount) {
			startPos = currentPos;
			targetPos = newTargetPos;
			duration = Max(stepCount, 1);
			elapsed = 0;
			type = easingType;
			active = true;
		}

		Vec2 update() {
			if (!active) {
				return targetPos;
			}

			const double t = elapsed / static_cast<double>(duration);
			const Vec2 result = startPos + (targetPos - startPos) * applyEasing(type, t);

			++elapsed;
			if (elapsed > duration) {
				active = false;
				return targetPos;
			}

			return result;
		}

		bool isActive() const {
			return active;
		}
	};

	inline EasingMoveType fromGameMakerEasingType(int32 type) {
		switch (type) {
		case 1:
			return EasingMoveType::EaseIn;
		case 2:
			return EasingMoveType::EaseOut;
		case 3:
			return EasingMoveType::EaseInOut;
		case 0:
		default:
			return EasingMoveType::Linear;
		}
	}
}
