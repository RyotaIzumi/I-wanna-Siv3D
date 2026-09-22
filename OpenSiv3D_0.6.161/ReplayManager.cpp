#include "ReplayManager.h"

#include <fstream>

namespace {
	constexpr const char* ReplayFilePath = "replays.dat";
	constexpr uint32 ReplayFileMagic = 0x52505749;
	constexpr uint32 ReplayFileVersion = 3;
	constexpr uint32 MaxStoredFrames = 60 * 60 * Global::FPS;

	template <class Type>
	void writeValue(std::ofstream& writer, const Type& value) {
		writer.write(reinterpret_cast<const char*>(&value), sizeof(Type));
	}

	template <class Type>
	bool readValue(std::ifstream& reader, Type& value) {
		return static_cast<bool>(reader.read(reinterpret_cast<char*>(&value), sizeof(Type)));
	}

	bool readReplay(std::ifstream& reader, uint32 version, Iwanna::ReplayData& replay) {
		int32 difficulty = 0;
		uint32 dateSize = 0;
		uint32 frameCount = 0;
		if (!readValue(reader, replay.chapter) || !readValue(reader, replay.startStep)
			|| !readValue(reader, replay.fps) || !readValue(reader, replay.randomSeed)
			|| !readValue(reader, difficulty) || !readValue(reader, dateSize) || 64 < dateSize) return false;

		std::string recordedAt(dateSize, '\0');
		if (dateSize != 0 && !reader.read(recordedAt.data(), dateSize)) return false;

		replay.difficulty = static_cast<Global::Difficulty>(Clamp(difficulty, 0, 2));
		replay.recordedAt = Unicode::FromUTF8(recordedAt);
		if (3 <= version) {
			uint32 pathSize = 0;
			if (!readValue(reader, pathSize) || 512 < pathSize) return false;
			std::string screenshotPath(pathSize, '\0');
			if (pathSize != 0 && !reader.read(screenshotPath.data(), pathSize)) return false;
			replay.screenshotPath = Unicode::FromUTF8(screenshotPath);
			const String screenCapturePath = U"Screenshot/" + replay.screenshotPath;
			if (!replay.screenshotPath.isEmpty()
				&& !FileSystem::Exists(replay.screenshotPath)
				&& FileSystem::Exists(screenCapturePath)) {
				replay.screenshotPath = screenCapturePath;
			}
		}
		if (!readValue(reader, frameCount) || MaxStoredFrames < frameCount) return false;
		replay.frames.reserve(frameCount);
		replay.frameSteps.reserve(frameCount);
		for (uint32 i = 0; i < frameCount; ++i) {
			uint8 bits = 0;
			int32 step = 0;
			if (!readValue(reader, bits) || !readValue(reader, step)) return false;
			Iwanna::ReplayInputFrame frame;
			frame.leftPressed = (bits & 0x01) != 0;
			frame.rightPressed = (bits & 0x02) != 0;
			frame.jumpDown = (bits & 0x04) != 0;
			frame.jumpUp = (bits & 0x08) != 0;
			frame.shootDown = (bits & 0x10) != 0;
			replay.frames << frame;
			replay.frameSteps << step;
		}
		return replay.isValid();
	}

	void writeReplay(std::ofstream& writer, const Iwanna::ReplayData& replay) {
		writeValue(writer, replay.chapter);
		writeValue(writer, replay.startStep);
		writeValue(writer, replay.fps);
		writeValue(writer, replay.randomSeed);
		writeValue(writer, static_cast<int32>(replay.difficulty));
		const std::string recordedAt = Unicode::ToUTF8(replay.recordedAt);
		writeValue(writer, static_cast<uint32>(recordedAt.size()));
		writer.write(recordedAt.data(), recordedAt.size());
		const std::string screenshotPath = Unicode::ToUTF8(replay.screenshotPath);
		writeValue(writer, static_cast<uint32>(screenshotPath.size()));
		writer.write(screenshotPath.data(), screenshotPath.size());
		writeValue(writer, static_cast<uint32>(replay.frames.size()));
		for (size_t i = 0; i < replay.frames.size(); ++i) {
			const auto& frame = replay.frames[i];
			const uint8 bits = (frame.leftPressed ? 0x01 : 0) | (frame.rightPressed ? 0x02 : 0)
				| (frame.jumpDown ? 0x04 : 0) | (frame.jumpUp ? 0x08 : 0) | (frame.shootDown ? 0x10 : 0);
			writeValue(writer, bits);
			writeValue(writer, replay.frameSteps[i]);
		}
	}
}

namespace Iwanna {
	ReplayManager::ReplayManager() { load(); }

	void ReplayManager::beginRecording(int32 chapter, int32 startStep, Global::Difficulty difficulty) {
		const uint64 seed = RandomUint64();
		Reseed(seed);
		recordingReplay = ReplayData{};
		recordingReplay.chapter = chapter;
		recordingReplay.startStep = startStep;
		recordingReplay.fps = Global::FPS;
		recordingReplay.randomSeed = seed;
		recordingReplay.difficulty = difficulty;
		recordingReplay.recordedAt = DateTime::Now().format(U"yyyy/MM/dd HH:mm:ss");
		recording = true;
	}

	void ReplayManager::cancelRecording() { recording = false; recordingReplay = ReplayData{}; }
	void ReplayManager::recordFrame(const ReplayInputFrame& input, int32 step) {
		if (!recording) return;
		recordingReplay.frames << input;
		recordingReplay.frameSteps << step;
	}
	void ReplayManager::finishRecording() {
		if (!recording || !recordingReplay.isValid()) { cancelRecording(); return; }
		replayHistory.insert(replayHistory.begin(), recordingReplay);
		String removedScreenshotPath;
		if (MaxReplayCount < replayHistory.size()) {
			removedScreenshotPath = replayHistory.back().screenshotPath;
			replayHistory.resize(MaxReplayCount);
		}
		recording = false;
		save();
		removeScreenshotIfUnused(removedScreenshotPath);
	}
	String ReplayManager::finishRecordingWithScreenshot() {
		if (!recording || !recordingReplay.isValid()) {
			cancelRecording();
			return {};
		}

		const String capturePath = U"replay_screenshots/replay_"
			+ Format(recordingReplay.randomSeed) + U".png";
		recordingReplay.screenshotPath = U"Screenshot/" + capturePath;
		finishRecording();
		return capturePath;
	}
	bool ReplayManager::isRecording() const { return recording; }

	size_t ReplayManager::getReplayCount() const { return replayHistory.size(); }
	const ReplayData* ReplayManager::getReplay(size_t index) const { return index < replayHistory.size() ? &replayHistory[index] : nullptr; }
	size_t ReplayManager::getFavoriteReplayCount() const { return favoriteReplays.size(); }
	const ReplayData* ReplayManager::getFavoriteReplay(size_t index) const { return index < favoriteReplays.size() ? &favoriteReplays[index] : nullptr; }
	bool ReplayManager::isReplayFavorite(size_t index) const {
		const ReplayData* replay = getReplay(index);
		return replay && favoriteReplays.any([replay](const ReplayData& favorite) {
			return favorite.randomSeed == replay->randomSeed && favorite.recordedAt == replay->recordedAt;
		});
	}
	bool ReplayManager::canAddReplayToFavorites(size_t index) const {
		return getReplay(index) && favoriteReplays.size() < MaxFavoriteReplayCount && !isReplayFavorite(index);
	}
	void ReplayManager::addReplayToFavorites(size_t index) {
		if (!canAddReplayToFavorites(index)) return;
		favoriteReplays << replayHistory[index];
		save();
	}
	void ReplayManager::removeFavoriteReplay(size_t index) {
		if (favoriteReplays.size() <= index) return;
		const String screenshotPath = favoriteReplays[index].screenshotPath;
		favoriteReplays.erase(favoriteReplays.begin() + index);
		save();
		removeScreenshotIfUnused(screenshotPath);
	}

	bool ReplayManager::isScreenshotReferenced(const String& path) const {
		if (path.isEmpty()) return false;
		const auto referencesPath = [&path](const ReplayData& replay) { return replay.screenshotPath == path; };
		return replayHistory.any(referencesPath) || favoriteReplays.any(referencesPath);
	}

	void ReplayManager::removeScreenshotIfUnused(const String& path) const {
		if (!path.isEmpty() && !isScreenshotReferenced(path) && FileSystem::Exists(path)) {
			FileSystem::Remove(path);
		}
	}

	void ReplayManager::beginPlayback(const ReplayData& replay) {
		playbackReplay = replay;
		playbackFrame = 0;
		slowFrameSkip = false;
		Reseed(playbackReplay.randomSeed);
	}
	const ReplayData& ReplayManager::getPlaybackReplay() const { return playbackReplay; }
	bool ReplayManager::hasPlaybackFrame() const { return playbackReplay.isValid() && static_cast<size_t>(playbackFrame) < playbackReplay.frames.size(); }
	const ReplayInputFrame& ReplayManager::getPlaybackInput() const { return playbackReplay.frames[playbackFrame]; }
	int32 ReplayManager::getPlaybackStep() const { return playbackReplay.frameSteps[playbackFrame]; }
	void ReplayManager::advancePlaybackFrame() { if (hasPlaybackFrame()) ++playbackFrame; }
	bool ReplayManager::shouldAdvancePlayback(bool slowPlayback) {
		if (!slowPlayback) { slowFrameSkip = false; return true; }
		slowFrameSkip = !slowFrameSkip;
		return !slowFrameSkip;
	}

	void ReplayManager::load() {
		std::ifstream reader(ReplayFilePath, std::ios::binary);
		uint32 magic = 0, version = 0, replayCount = 0;
		if (!reader || !readValue(reader, magic) || !readValue(reader, version) || !readValue(reader, replayCount)
			|| magic != ReplayFileMagic || (version < 1 || ReplayFileVersion < version) || MaxReplayCount < replayCount) return;
		Array<ReplayData> loaded;
		for (uint32 i = 0; i < replayCount; ++i) { ReplayData replay; if (!readReplay(reader, version, replay)) return; loaded << std::move(replay); }
		Array<ReplayData> favorites;
		if (2 <= version) {
			uint32 count = 0;
			if (!readValue(reader, count) || MaxFavoriteReplayCount < count) return;
			for (uint32 i = 0; i < count; ++i) { ReplayData replay; if (!readReplay(reader, version, replay)) return; favorites << std::move(replay); }
		}
		replayHistory = std::move(loaded);
		favoriteReplays = std::move(favorites);
	}

	void ReplayManager::save() const {
		std::ofstream writer(ReplayFilePath, std::ios::binary | std::ios::trunc);
		if (!writer) return;
		writeValue(writer, ReplayFileMagic);
		writeValue(writer, ReplayFileVersion);
		writeValue(writer, static_cast<uint32>(replayHistory.size()));
		for (const auto& replay : replayHistory) writeReplay(writer, replay);
		writeValue(writer, static_cast<uint32>(favoriteReplays.size()));
		for (const auto& replay : favoriteReplays) writeReplay(writer, replay);
	}
}
