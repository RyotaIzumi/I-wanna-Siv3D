#pragma once
#include <Siv3D.hpp>
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager/AvoidanceManager.h"
#include "SaveData.h"

namespace Iwanna {
	class MainGame {
	private:
		StockNearGameObjects stockNearGameObjects;

		Audio audio;

		AvoidanceManager avoidanceManager;
		SaveData saveData;
		int32 lastSelectedChapter = 1;
		bool wasPlayerDead = false;
		bool shouldUpdateHighestEndurance = true;
		bool practiceLimitReached = false;
		bool isTutorial = false;
		bool canDebugChangeDifficulty = false;
		Optional<int32> practiceLimitStep;
		double saveTimerSec = 0.0;

		Optional<int32> getPracticeLimitStep() const;
		Optional<int32> getTrialLimitStep() const;
		Optional<int32> getStepLimitStep() const;
		void savePeriodically(double deltaTimeSec);

	public:
		MainGame();
		~MainGame();

		void startGame(int32 chapter = 1);
		void startTutorial();
		int32 getLastSelectedChapter() const;
		const SaveData& getSaveData() const;
		double getEnduranceLengthSec() const;
		bool canStartAvoidance() const;
		bool canChangeDifficulty() const;
		void setDifficulty(Global::Difficulty difficulty);
		void setBgmVolume(double volume);
		void setSeVolume(double volume);
		void updateGame();
		void debugGame();
		void drawGame();

		void playBgm(int32 chapter);
		void stopBgm();

		void pauseBgm();
	};
}
