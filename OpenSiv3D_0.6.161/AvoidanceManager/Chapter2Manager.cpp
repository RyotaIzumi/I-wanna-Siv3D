#include "AvoidanceManager.h"

namespace Iwanna {
	// step : 840 〜 1319
	void AvoidanceManager::chapter2() {
		Timeline timeline(previousStep, step);

		const auto acceleratingCherry = []() {
			auto cherry = std::make_shared<Cherry>();
			cherry->setBehavior([](Cherry& self, int32) {
				self.speed = Min(self.speed + 0.035, 5.0);
			});
			return cherry;
		};

		const auto waveCherry = []() {
			auto cherry = std::make_shared<Cherry>();
			cherry->setBehavior([](Cherry& self, int32 age) {
				self.dir += Math::Sin(age * 0.12) * 2.4;
			});
			return cherry;
		};

		const auto homingCherry = [player = gameObjects.player]() {
			auto cherry = std::make_shared<Cherry>();
			cherry->setBehavior([player](Cherry& self, int32) {
				const Vec2 diff = player->pos - self.pos;
				const double targetDir = Math::ToDegrees(Atan2(-diff.y, diff.x));
				double delta = targetDir - self.dir;

				while (delta > 180.0) delta -= 360.0;
				while (delta < -180.0) delta += 360.0;

				self.dir += delta * 0.045;
			});
			return cherry;
		};

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
	}
}
