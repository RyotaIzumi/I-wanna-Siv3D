#pragma once
#include <Siv3D.hpp>
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager/AvoidanceManager.h"
#include "SaveData.h"
#include "Replay.h"

namespace Iwanna {
	class MainGame {
	private:
		static constexpr size_t MaxReplayCount = 5;
		static constexpr size_t MaxFavoriteReplayCount = 20;

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
		Array<ReplayData> replayHistory;
		Array<ReplayData> favoriteReplays;
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
		void loadReplayHistory();
		void saveReplayHistory() const;
		void startReplayData(const ReplayData& replay, int32 startChapter);
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
		size_t getReplayCount() const;
		const ReplayData* getReplay(size_t index) const;
		bool canStartReplay(size_t index, int32 startChapter = 1) const;
		void startReplay(size_t index, int32 startChapter = 1);
		size_t getFavoriteReplayCount() const;
		const ReplayData* getFavoriteReplay(size_t index) const;
		bool isReplayFavorite(size_t index) const;
		bool canAddReplayToFavorites(size_t index) const;
		void addReplayToFavorites(size_t index);
		void removeFavoriteReplay(size_t index);
		bool canStartFavoriteReplay(size_t index, int32 startChapter = 1) const;
		void startFavoriteReplay(size_t index, int32 startChapter = 1);
		bool canStartLastReplay() const;
		void startLastReplay();
		void returnToStartMenu();
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
