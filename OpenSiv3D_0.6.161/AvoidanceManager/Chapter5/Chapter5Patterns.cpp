#include "Chapter5Patterns.h"
#include "../AvoidanceManager.h"
#include "../../GameObject/Cherries/Chapter5/Chapter5CherryBehaviors.h"

namespace Iwanna::Chapter5Patterns {
	namespace {
		Cherry::Settings makeCherrySettings(const ColorF& color, Cherry::Behavior behavior) {
			Cherry::Settings settings;
			settings.textureName = U"sprCherryAllWhite";
			settings.color = color;
			settings.behavior = std::move(behavior);
			return settings;
		}
	}

	void spawnRadial(AvoidanceManager& manager, const Vec2& origin, const RadialSettings& settings) {
		for (int32 i = 0; i < settings.count; ++i) {
			const double angle = settings.angleOffset + 360.0 * i / settings.count;
			manager.createCherry(origin, makeCherrySettings(settings.color,
				Chapter5CherryBehaviors::makeLinear(settings.speed, angle)));
		}
	}

	void spawnAimedFan(AvoidanceManager& manager, const Vec2& origin, const AimedFanSettings& settings) {
		const auto player = manager.getPlayer();
		if (!player) {
			return;
		}

		// Aim once when firing; movement does not retain the player or manager.
		const Vec2 delta = player->pos - origin;
		const double aimAngle = Math::ToDegrees(Atan2(-delta.y, delta.x));
		for (int32 i = 0; i < settings.count; ++i) {
			const double offset = (i - (settings.count - 1) * 0.5) * settings.angleSpacing;
			manager.createCherry(origin, makeCherrySettings(settings.color,
				Chapter5CherryBehaviors::makeLinear(settings.speed, aimAngle + offset)));
		}
	}

	void spawnWaveRow(AvoidanceManager& manager, const WaveSettings& settings) {
		for (int32 i = 0; i < settings.count; ++i) {
			const Vec2 origin{ settings.startX + settings.spacing * i, settings.startY };
			manager.createCherry(origin, makeCherrySettings(settings.color,
				Chapter5CherryBehaviors::makeSineFall(origin, settings.fallSpeed,
					settings.amplitude, settings.periodFrames, settings.phase)));
		}
	}
}
