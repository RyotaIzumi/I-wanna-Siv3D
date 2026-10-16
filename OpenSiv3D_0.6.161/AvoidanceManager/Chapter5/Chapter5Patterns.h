#pragma once
#include <Siv3D.hpp>

namespace Iwanna {
	class AvoidanceManager;

	namespace Chapter5Patterns {
		struct RadialSettings {
			int32 count = 12;
			double speed = 2.4;
			double angleOffset = 0.0;
			ColorF color = ColorF{ 0.95, 0.25, 0.35 };
		};

		struct AimedFanSettings {
			int32 count = 5;
			double speed = 3.0;
			double angleSpacing = 12.0;
			ColorF color = ColorF{ 0.15, 0.55, 0.95 };
		};

		struct WaveSettings {
			int32 count = 9;
			double startX = 80.0;
			double spacing = 80.0;
			double startY = 48.0;
			double fallSpeed = 2.2;
			double amplitude = 20.0;
			int32 periodFrames = 120;
			double phase = 0.0;
			ColorF color = ColorF{ 0.15, 0.75, 0.4 };
		};

		void spawnRadial(AvoidanceManager& manager, const Vec2& origin,
			const RadialSettings& settings = RadialSettings{});
		void spawnAimedFan(AvoidanceManager& manager, const Vec2& origin,
			const AimedFanSettings& settings = AimedFanSettings{});
		void spawnWaveRow(AvoidanceManager& manager,
			const WaveSettings& settings = WaveSettings{});
	}
}
