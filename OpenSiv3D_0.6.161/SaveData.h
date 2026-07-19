#pragma once
#include <Siv3D.hpp>

namespace Iwanna {
	struct SaveData {
		int32 deathCount = 0;
		double playTimeSec = 0.0;
		int32 highestChapter = 1;
		double highestEnduranceSec = 0.0;

		void load();
		void save() const;
		void addDeath();
		void addPlayTime(double seconds);
		void updateHighestChapter(int32 chapter);
		void updateHighestEnduranceSec(double seconds);
	};
}
