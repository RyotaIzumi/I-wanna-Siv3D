#pragma once
#include <Siv3D.hpp>
#include "Replay.h"

namespace Iwanna {
	class ReplayManager {
	private:
		static constexpr size_t MaxReplayCount = 5;
		static constexpr size_t MaxFavoriteReplayCount = 20;

		ReplayData recordingReplay;
		ReplayData playbackReplay;
		Array<ReplayData> replayHistory;
		Array<ReplayData> favoriteReplays;
		int32 playbackFrame = 0;
		bool recording = false;
		bool slowFrameSkip = false;

		void load();
		void save() const;

	public:
		ReplayManager();

		void beginRecording(int32 chapter, int32 startStep, Global::Difficulty difficulty);
		void cancelRecording();
		void recordFrame(const ReplayInputFrame& input, int32 step);
		void finishRecording();
		bool isRecording() const;

		size_t getReplayCount() const;
		const ReplayData* getReplay(size_t index) const;
		size_t getFavoriteReplayCount() const;
		const ReplayData* getFavoriteReplay(size_t index) const;
		bool isReplayFavorite(size_t index) const;
		bool canAddReplayToFavorites(size_t index) const;
		void addReplayToFavorites(size_t index);
		void removeFavoriteReplay(size_t index);

		void beginPlayback(const ReplayData& replay);
		const ReplayData& getPlaybackReplay() const;
		bool hasPlaybackFrame() const;
		const ReplayInputFrame& getPlaybackInput() const;
		int32 getPlaybackStep() const;
		void advancePlaybackFrame();
		bool shouldAdvancePlayback(bool slowPlayback);
	};
}
