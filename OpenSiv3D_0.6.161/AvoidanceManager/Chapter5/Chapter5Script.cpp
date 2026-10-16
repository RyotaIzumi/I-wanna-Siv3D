#include "Chapter5Script.h"
#include "Chapter5Patterns.h"
#include "../Timeline.h"

namespace Iwanna {
	void Chapter5Script::update(AvoidanceManager& manager, const Timeline& timeline) const {
		// Schedule times are music steps relative to the start of Chapter 5.
		timeline.at(20, [&]() {
			Chapter5Patterns::spawnRadial(manager, { 400, 160 });
		});

		timeline.every(24, 44, 140, [&](int32 localStep) {
			Chapter5Patterns::RadialSettings settings;
			settings.angleOffset = (localStep / 24) * 12.0;
			Chapter5Patterns::spawnRadial(manager, { 400, 160 }, settings);
		});

		timeline.every(20, 180, 280, [&](int32 localStep) {
			const bool fromLeft = ((localStep / 20) % 2 == 0);
			Chapter5Patterns::spawnAimedFan(manager, { fromLeft ? 96.0 : 704.0, 160 });
		});

		timeline.every(24, 320, 440, [&](int32 localStep) {
			Chapter5Patterns::WaveSettings settings;
			settings.phase = (localStep / 24) * Math::Pi / 2.0;
			Chapter5Patterns::spawnWaveRow(manager, settings);
		});
	}
}
