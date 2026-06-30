#include "AvoidanceManager.h"

namespace Iwanna {
	// step :0 〜 839
	void AvoidanceManager::chapter1() {
		Timeline timeline(previousStep, step);
		const auto cherry = []() { return std::make_shared<Cherry>(); };

		timeline.at(50, [&] {
			createCherrySpread(Vec2{ 400,300 }, 24, 2.4, cherry);
		});

		timeline.every(50, 100, 300, [&](int32 localStep) {
			const double y = 120 + (localStep / 50) * 80;
			createCherrySpread(Vec2{ 80,y }, 10, 2.0, cherry);
			createCherrySpread(Vec2{ 720,y }, 10, 2.0, cherry);
		});

		timeline.during(350, 500, [&](int32 localStep) {
			if (localStep % 15 != 0) {
				return;
			}

			const double x = 160 + localStep * 3.2;
			createCherrySpread(Vec2{ x,80 }, 8, 2.6, cherry);
		});
	}
}
