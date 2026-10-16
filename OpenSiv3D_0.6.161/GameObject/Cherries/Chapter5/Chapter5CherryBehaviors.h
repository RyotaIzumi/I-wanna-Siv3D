#pragma once
#include "../../Cherry.h"

namespace Iwanna::Chapter5CherryBehaviors {
	// Angles follow Cherry's convention: 0 = right, 90 = up, in degrees.
	Cherry::Behavior makeLinear(double speed, double angle, int32 lifetimeFrames = 360);
	Cherry::Behavior makeSineFall(const Vec2& origin, double fallSpeed, double amplitude,
		int32 periodFrames, double phase = 0.0, int32 lifetimeFrames = 360);
}
