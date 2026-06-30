#include "AvoidanceManager.h"

namespace Iwanna {
	// step : 840 〜 1319
	void AvoidanceManager::chapter2() {
		Timeline timeline(previousStep, step);

		const auto acceleratingCherry = makeCherryFactory(
			U"sprCherryAllWhite",
			ColorF(1.0, 0.25, 0.25),
			[](Cherry& self, int32) {
				self.speed = Min(self.speed + 0.035, 5.0);
			});

		const auto waveCherry = makeCherryFactory(
			U"sprCherryWhite",
			ColorF(0.25, 0.55, 1.0),
			[](Cherry& self, int32 age) {
				self.dir += Math::Sin(age * 0.12) * 2.4;
			});

		const auto homingCherry = makeCherryFactory(
			U"sprCherryFrameWhite",
			ColorF(0.45, 1.0, 0.35),
			[player = gameObjects.player](Cherry& self, int32) {
				const Vec2 diff = player->pos - self.pos;
				const double targetDir = Math::ToDegrees(Atan2(-diff.y, diff.x));
				double delta = targetDir - self.dir;

				while (delta > 180.0) delta -= 360.0;
				while (delta < -180.0) delta += 360.0;

				self.dir += delta * 0.045;
			});

		const auto rainbowCherry = makeCherryFactory(
			U"sprCherryAllWhite",
			Palette::White,
			[](Cherry& self, int32 age) {
				self.setColor(ColorF(HSV(age * 5.0, 0.85, 1.0)));
				self.dir += 1.0;
			});

		timeline.at(850, [&] {
			createCherrySpread(Vec2{ 400,300 }, 18, 0.6, acceleratingCherry);
		});

		timeline.every(40, 920, 1080, [&](int32 localStep) {
			const double x = 160 + localStep * 2.4;
			createCherrySpread(Vec2{ x,80 }, 7, 2.2, waveCherry);
		});

		timeline.every(35, 1120, 1260, [&](int32 localStep) {
			const double y = 120 + (localStep / 35) * 70;
			createCherrySpread(Vec2{ 60,y }, 5, 2.4, homingCherry);
			createCherrySpread(Vec2{ 740,y }, 5, 2.4, homingCherry);
		});

		timeline.at(1290, [&] {
			createCherrySpread(Vec2{ 400,300 }, 36, 2.8, rainbowCherry);
		});
	}
}
