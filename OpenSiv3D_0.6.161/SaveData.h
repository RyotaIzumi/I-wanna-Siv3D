#pragma once
#include <Siv3D.hpp>
#include <array>
#include "Global.h"

namespace Iwanna {
	struct SaveData {
		static constexpr int32 ChapterCount = 6;
		static constexpr int32 AchievementCount = 7;

		int32 deathCount = 0;
		double playTimeSec = 0.0;
		int32 highestChapter = 1;
		double highestEnduranceSec = 0.0;
		double bgmVolume = 1.0;
		double seVolume = 1.0;
		Global::Difficulty difficulty = Global::Difficulty::Unselected;
		bool hasStartedAvoidance = false;
		std::array<int32, ChapterCount> chapterDeathCounts{};
		std::array<String, AchievementCount> achievementUnlockedAt{};

		void load();
		void save() const;
		void addDeath(int32 chapter);
		void addPlayTime(double seconds);
		void updateHighestChapter(int32 chapter);
		void updateHighestEnduranceSec(double seconds);
		bool unlockAchievement(int32 index);
	};
}
