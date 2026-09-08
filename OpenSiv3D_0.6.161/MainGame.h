#pragma once
#include <Siv3D.hpp>
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager/AvoidanceManager.h"
#include "SaveData.h"
#include "Replay.h"

namespace Iwanna {
	class MainGame {
	private:
		enum class PlayMode {
			Normal,
			Replay,
		};

		StockNearGameObjects stockNearGameObjects;

		Audio audio;

		AvoidanceManager avoidanceManager;
		SaveData saveData;
		ReplayData recordingReplay;
		ReplayData playbackReplay;
		ReplayData lastReplay;
		int32 lastSelectedChapter = 1;
		int32 replayFrame = 0;
		bool wasPlayerDead = false;
		bool shouldUpdateHighestEndurance = true;
		bool practiceLimitReached = false;
		bool isTutorial = false;
		bool canDebugChangeDifficulty = false;
		bool isRecordingReplay = false;
		PlayMode playMode = PlayMode::Normal;
		Optional<int32> practiceLimitStep;
		double saveTimerSec = 0.0;

		Optional<int32> getPracticeLimitStep() const;
		Optional<int32> getTrialLimitStep() const;
		Optional<int32> getStepLimitStep() const;
		int32 getChapterStartStep(int32 chapter) const;
		bool isPracticeMode() const;
		bool isReplayMode() const;
		void togglePlayerMuteki();
		void savePeriodically(double deltaTimeSec);
		void beginReplayRecording(int32 chapter);
		void finishReplayRecording();
		void updateNormalGame();
		void updateReplayGame();

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
		bool canStartLastReplay() const;
		void startLastReplay();
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
