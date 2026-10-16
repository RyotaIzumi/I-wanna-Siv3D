#include "Chapter5CherryBehaviors.h"

namespace Iwanna::Chapter5CherryBehaviors {
	Cherry::Behavior makeLinear(double speed, double angle, int32 lifetimeFrames) {
		return [=](Cherry& cherry, int32 age) {
			cherry.speed = speed;
			cherry.dir = angle;
			cherry.isDelete = (age >= lifetimeFrames);
		};
	}

	Cherry::Behavior makeSineFall(const Vec2& origin, double fallSpeed, double amplitude,
		int32 periodFrames, double phase, int32 lifetimeFrames) {
		const double angularSpeed = Math::TwoPi / Max(periodFrames, 1);
		return [=](Cherry& cherry, int32 age) {
			// Direct position control bypasses Cherry's speed/dir movement.
			cherry.speed = 0.0;
			const double elapsed = age + 1.0;
			cherry.pos = origin + Vec2{
				amplitude * (Sin(phase + angularSpeed * elapsed) - Sin(phase)),
				fallSpeed * elapsed };
			cherry.isDelete = (age >= lifetimeFrames);
		};
	}
}
