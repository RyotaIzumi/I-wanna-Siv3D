#include "MainGame.h"

#include "Audio/AudioAsset.h"
#include <fstream>

namespace {
	constexpr const char* ReplayFilePath = "replays.dat";
	constexpr uint32 ReplayFileMagic = 0x52505749;
	constexpr uint32 ReplayFileVersion = 2;
	constexpr uint32 MaxStoredFrames = 60 * 60 * Global::FPS;

	template <class Type>
	void writeValue(std::ofstream& writer, const Type& value) {
		writer.write(reinterpret_cast<const char*>(&value), sizeof(Type));
	}

	template <class Type>
	bool readValue(std::ifstream& reader, Type& value) {
		return static_cast<bool>(reader.read(reinterpret_cast<char*>(&value), sizeof(Type)));
	}

	bool readReplay(std::ifstream& reader, Iwanna::ReplayData& replay) {
		int32 difficulty = 0;
		uint32 dateSize = 0;
		uint32 frameCount = 0;
		if (!readValue(reader, replay.chapter)
			|| !readValue(reader, replay.startStep)
			|| !readValue(reader, replay.fps)
			|| !readValue(reader, replay.randomSeed)
			|| !readValue(reader, difficulty)
			|| !readValue(reader, dateSize)
			|| 64 < dateSize) {
			return false;
		}

		std::string recordedAt(dateSize, '\0');
		if (dateSize != 0 && !reader.read(recordedAt.data(), dateSize)) {
			return false;
		}
		if (!readValue(reader, frameCount) || MaxStoredFrames < frameCount) {
			return false;
		}

		replay.difficulty = static_cast<Global::Difficulty>(Clamp(difficulty, 0, 2));
		replay.recordedAt = Unicode::FromUTF8(recordedAt);
		replay.frames.reserve(frameCount);
		replay.frameSteps.reserve(frameCount);
		for (uint32 frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			uint8 inputBits = 0;
			int32 step = 0;
			if (!readValue(reader, inputBits) || !readValue(reader, step)) {
				return false;
			}

			Iwanna::ReplayInputFrame frame;
			frame.leftPressed = (inputBits & 0x01) != 0;
			frame.rightPressed = (inputBits & 0x02) != 0;
			frame.jumpDown = (inputBits & 0x04) != 0;
			frame.jumpUp = (inputBits & 0x08) != 0;
			frame.shootDown = (inputBits & 0x10) != 0;
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
		const int32 difficulty = static_cast<int32>(replay.difficulty);
		writeValue(writer, difficulty);
		const std::string recordedAt = Unicode::ToUTF8(replay.recordedAt);
		const uint32 dateSize = static_cast<uint32>(recordedAt.size());
		writeValue(writer, dateSize);
		writer.write(recordedAt.data(), dateSize);
		const uint32 frameCount = static_cast<uint32>(replay.frames.size());
		writeValue(writer, frameCount);

		for (size_t frameIndex = 0; frameIndex < replay.frames.size(); ++frameIndex) {
			const Iwanna::ReplayInputFrame& frame = replay.frames[frameIndex];
			const uint8 inputBits =
				(frame.leftPressed ? 0x01 : 0)
				| (frame.rightPressed ? 0x02 : 0)
				| (frame.jumpDown ? 0x04 : 0)
				| (frame.jumpUp ? 0x08 : 0)
				| (frame.shootDown ? 0x10 : 0);
			writeValue(writer, inputBits);
			writeValue(writer, replay.frameSteps[frameIndex]);
		}
	}
}

namespace Iwanna {
	MainGame::MainGame() {
		saveData.load();
		loadReplayHistory();
		Global::bgmVolume = saveData.bgmVolume;
		Global::seVolume = saveData.seVolume;
		Global::difficulty = saveData.difficulty;
	}

	MainGame::~MainGame() {
		finishReplayRecording();
		saveData.save();
	}

	void MainGame::startGame(int32 chapter) {
		if (!canStartAvoidance()) {
			return;
		}

		playMode = PlayMode::Normal;
		isTutorial = false;
		lastSelectedChapter = Clamp(chapter, 1, 6);
		wasPlayerDead = false;
		shouldUpdateHighestEndurance = (lastSelectedChapter == 1);
		practiceLimitReached = false;
		practiceLimitStep = getStepLimitStep();
		saveData.hasStartedAvoidance = true;
		saveData.unlockAchievement(0);
		saveData.updateHighestChapter(lastSelectedChapter);
		saveData.save();
		beginReplayRecording(lastSelectedChapter);
		avoidanceManager.setUpObjects(lastSelectedChapter);
		playBgm(lastSelectedChapter);
	}

	void MainGame::startTutorial() {
		playMode = PlayMode::Normal;
		isTutorial = true;
		wasPlayerDead = false;
		isRecordingReplay = false;
		shouldUpdateHighestEndurance = false;
		practiceLimitReached = false;
		practiceLimitStep = none;
		audio.stop();
		avoidanceManager.setUpTutorialObjects();
	}

	int32 MainGame::getLastSelectedChapter() const {
		return lastSelectedChapter;
	}

	const SaveData& MainGame::getSaveData() const {
		return saveData;
	}

	double MainGame::getEnduranceLengthSec() const {
		return AudioAsset{ U"sndHibana" }.lengthSec();
	}

	bool MainGame::canStartAvoidance() const {
		return saveData.difficulty != Global::Difficulty::Unselected;
	}

	bool MainGame::canChangeDifficulty() const {
		return !saveData.hasStartedAvoidance || canDebugChangeDifficulty;
	}

	size_t MainGame::getReplayCount() const {
		return replayHistory.size();
	}

	const ReplayData* MainGame::getReplay(size_t index) const {
		return (index < replayHistory.size()) ? &replayHistory[index] : nullptr;
	}

	bool MainGame::canStartReplay(size_t index) const {
		const ReplayData* replay = getReplay(index);
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& replay
			&& replay->isValid();
	}

	void MainGame::startReplay(size_t index) {
		if (!canStartReplay(index)) {
			return;
		}

		startReplayData(replayHistory[index]);
	}

	size_t MainGame::getFavoriteReplayCount() const {
		return favoriteReplays.size();
	}

	const ReplayData* MainGame::getFavoriteReplay(size_t index) const {
		return (index < favoriteReplays.size()) ? &favoriteReplays[index] : nullptr;
	}

	bool MainGame::isReplayFavorite(size_t index) const {
		const ReplayData* replay = getReplay(index);
		if (!replay) {
			return false;
		}

		return favoriteReplays.any([replay](const ReplayData& favorite) {
			return favorite.randomSeed == replay->randomSeed
				&& favorite.recordedAt == replay->recordedAt;
		});
	}

	bool MainGame::canAddReplayToFavorites(size_t index) const {
		return getReplay(index)
			&& favoriteReplays.size() < MaxFavoriteReplayCount
			&& !isReplayFavorite(index);
	}

	void MainGame::addReplayToFavorites(size_t index) {
		if (!canAddReplayToFavorites(index)) {
			return;
		}

		favoriteReplays << replayHistory[index];
		saveReplayHistory();
	}

	bool MainGame::canStartFavoriteReplay(size_t index) const {
		const ReplayData* replay = getFavoriteReplay(index);
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& replay
			&& replay->isValid();
	}

	void MainGame::startFavoriteReplay(size_t index) {
		if (!canStartFavoriteReplay(index)) {
			return;
		}

		startReplayData(favoriteReplays[index]);
	}

	void MainGame::startReplayData(const ReplayData& replay) {

		stopBgm();
		playMode = PlayMode::Replay;
		isTutorial = false;
		isRecordingReplay = false;
		wasPlayerDead = false;
		practiceLimitReached = false;
		practiceLimitStep = none;
		replayFrame = 0;
		playbackReplay = replay;
		Global::difficulty = playbackReplay.difficulty;
		Reseed(playbackReplay.randomSeed);
		avoidanceManager.setUpObjects(playbackReplay.chapter);
		audio = AudioAsset{ U"sndHibana" };
		audio.setVolume(saveData.bgmVolume);
		audio.seekTime(SecondsF(static_cast<double>(playbackReplay.startStep) / static_cast<double>(Global::FPS)));
		audio.play();
	}

	bool MainGame::canStartLastReplay() const {
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& wasPlayerDead
			&& !replayHistory.isEmpty()
			&& replayHistory.front().isValid();
	}

	void MainGame::startLastReplay() {
		if (!canStartLastReplay()) {
			return;
		}

		startReplay(0);
	}

	void MainGame::returnToStartMenu() {
		const bool shouldKeepLastReplaySelectable = isReplayMode() && !replayHistory.isEmpty();
		finishReplayRecording();
		stopBgm();
		playMode = PlayMode::Normal;
		isTutorial = false;
		isRecordingReplay = false;
		if (shouldKeepLastReplaySelectable) {
			wasPlayerDead = true;
		}
	}

	void MainGame::setDifficulty(Global::Difficulty difficulty) {
		if (!canChangeDifficulty()) {
			return;
		}

		saveData.difficulty = difficulty;
		Global::difficulty = saveData.difficulty;
		saveData.save();
	}

	void MainGame::setBgmVolume(double volume) {
		saveData.bgmVolume = Clamp(volume, 0.0, 1.0);
		Global::bgmVolume = saveData.bgmVolume;
		audio.setVolume(saveData.bgmVolume);
		saveData.save();
	}

	void MainGame::setSeVolume(double volume) {
		saveData.seVolume = Clamp(volume, 0.0, 1.0);
		Global::seVolume = saveData.seVolume;
		saveData.save();
	}

	Optional<int32> MainGame::getPracticeLimitStep() const {
		if (shouldUpdateHighestEndurance) {
			return none;
		}

		switch (Clamp(saveData.highestChapter + 1, 1, 7)) {
		case 2: return Global::startStep_Chapter2 - 1;
		case 3: return Global::startStep_Chapter3 - 1;
		case 4: return Global::startStep_Chapter4 - 1;
		case 5: return Global::startStep_Chapter5 - 1;
		case 6: return Global::startStep_Chapter6 - 1;
		default: return none;
		}
	}

	Optional<int32> MainGame::getTrialLimitStep() const {
		switch (Clamp(Global::trialClearableChapter + 2, 1, 7)) {
		case 2: return Global::startStep_Chapter2 - 1;
		case 3: return Global::startStep_Chapter3 - 1;
		case 4: return Global::startStep_Chapter4 - 1;
		case 5: return Global::startStep_Chapter5 - 1;
		case 6: return Global::startStep_Chapter6 - 1;
		default: return none;
		}
	}

	Optional<int32> MainGame::getStepLimitStep() const {
		const Optional<int32> practiceLimit = getPracticeLimitStep();
		const Optional<int32> trialLimit = getTrialLimitStep();

		if (practiceLimit && trialLimit) {
			return Min(*practiceLimit, *trialLimit);
		}

		return practiceLimit ? practiceLimit : trialLimit;
	}

	int32 MainGame::getChapterStartStep(int32 chapter) const {
		switch (Clamp(chapter, 1, 6)) {
		case 1: return Global::startStep_Chapter1;
		case 2: return Global::startStep_Chapter2;
		case 3: return Global::startStep_Chapter3;
		case 4: return Global::startStep_Chapter4;
		case 5: return Global::startStep_Chapter5;
		case 6: return Global::startStep_Chapter6;
		default: return Global::startStep_Chapter1;
		}
	}

	bool MainGame::isPracticeMode() const {
		return !isTutorial && 2 <= lastSelectedChapter;
	}

	bool MainGame::isReplayMode() const {
		return playMode == PlayMode::Replay;
	}

	void MainGame::togglePlayerMuteki() {
		if (const auto player = avoidanceManager.getPlayer()) {
			player->setIsMuteki(!player->getIsMuteki());
		}
	}

	void MainGame::beginReplayRecording(int32 chapter) {
		const uint64 randomSeed = RandomUint64();
		Reseed(randomSeed);

		recordingReplay = ReplayData{};
		recordingReplay.chapter = Clamp(chapter, 1, 6);
		recordingReplay.startStep = getChapterStartStep(recordingReplay.chapter);
		recordingReplay.fps = Global::FPS;
		recordingReplay.randomSeed = randomSeed;
		recordingReplay.difficulty = Global::difficulty;
		recordingReplay.recordedAt = DateTime::Now().format(U"yyyy/MM/dd HH:mm:ss");
		recordingReplay.frames.clear();
		recordingReplay.frameSteps.clear();
		isRecordingReplay = true;
	}

	void MainGame::finishReplayRecording() {
		if (!isRecordingReplay || !recordingReplay.isValid()) {
			isRecordingReplay = false;
			return;
		}

		replayHistory.insert(replayHistory.begin(), recordingReplay);
		if (MaxReplayCount < replayHistory.size()) {
			replayHistory.resize(MaxReplayCount);
		}
		saveReplayHistory();
		isRecordingReplay = false;
	}

	void MainGame::loadReplayHistory() {
		std::ifstream reader(ReplayFilePath, std::ios::binary);
		uint32 magic = 0;
		uint32 version = 0;
		uint32 replayCount = 0;
		if (!reader
			|| !readValue(reader, magic)
			|| !readValue(reader, version)
			|| !readValue(reader, replayCount)
			|| magic != ReplayFileMagic
			|| (version != 1 && version != ReplayFileVersion)
			|| MaxReplayCount < replayCount) {
			return;
		}

		Array<ReplayData> loaded;
		for (uint32 replayIndex = 0; replayIndex < replayCount; ++replayIndex) {
			ReplayData replay;
			if (!readReplay(reader, replay)) {
				return;
			}
			loaded << std::move(replay);
		}

		Array<ReplayData> loadedFavorites;
		if (2 <= version) {
			uint32 favoriteCount = 0;
			if (!readValue(reader, favoriteCount) || MaxFavoriteReplayCount < favoriteCount) {
				return;
			}
			for (uint32 favoriteIndex = 0; favoriteIndex < favoriteCount; ++favoriteIndex) {
				ReplayData replay;
				if (!readReplay(reader, replay)) {
					return;
				}
				loadedFavorites << std::move(replay);
			}
		}

		replayHistory = std::move(loaded);
		favoriteReplays = std::move(loadedFavorites);
	}

	void MainGame::saveReplayHistory() const {
		std::ofstream writer(ReplayFilePath, std::ios::binary | std::ios::trunc);
		if (!writer) {
			return;
		}

		writeValue(writer, ReplayFileMagic);
		writeValue(writer, ReplayFileVersion);
		const uint32 replayCount = static_cast<uint32>(replayHistory.size());
		writeValue(writer, replayCount);

		for (const ReplayData& replay : replayHistory) {
			writeReplay(writer, replay);
		}

		const uint32 favoriteCount = static_cast<uint32>(favoriteReplays.size());
		writeValue(writer, favoriteCount);
		for (const ReplayData& replay : favoriteReplays) {
			writeReplay(writer, replay);
		}
	}

	void MainGame::updateGame() {
		if (isReplayMode()) {
			updateReplayGame();
			return;
		}

		updateNormalGame();
	}

	void MainGame::updateNormalGame() {
		if (isTutorial) {
			avoidanceManager.updateTutorial();
			return;
		}

		if (isPracticeMode() && Global::inputDebugMuteki.down()) {
			togglePlayerMuteki();
		}

		const bool wasAliveAtFrameStart = !wasPlayerDead;
		const double deltaTimeSec = Scene::DeltaTime();
		if (wasAliveAtFrameStart) {
			saveData.addPlayTime(deltaTimeSec);
			savePeriodically(deltaTimeSec);
		}

		const double enduranceSec = audio.posSec();
		int32 newStep = static_cast<int32>(enduranceSec * Global::FPS);
		if (practiceLimitStep && *practiceLimitStep <= newStep) {
			newStep = *practiceLimitStep;
			if (!practiceLimitReached) {
				audio.stop();
				practiceLimitReached = true;
				saveData.save();
			}
		}
		if (practiceLimitReached && practiceLimitStep) {
			newStep = *practiceLimitStep;
		}

		const ReplayInputFrame inputFrame = ReplayInputFrame::FromCurrentInput();
		if (isRecordingReplay && wasAliveAtFrameStart) {
			recordingReplay.frames << inputFrame;
			recordingReplay.frameSteps << newStep;
		}

		avoidanceManager.setStep(newStep);
		avoidanceManager.update(inputFrame);
		const int32 previousHighestChapter = saveData.highestChapter;
		const int32 activeChapter = avoidanceManager.getActiveChapter();
		bool shouldSave = false;

		if (previousHighestChapter < activeChapter) {
			for (int32 clearedChapter = previousHighestChapter; clearedChapter < activeChapter; ++clearedChapter) {
				shouldSave |= saveData.unlockAchievement(clearedChapter);
			}
		}

		saveData.updateHighestChapter(activeChapter);
		if (shouldUpdateHighestEndurance) {
			saveData.updateHighestEnduranceSec(enduranceSec);
		}
		if (shouldUpdateHighestEndurance
			&& getEnduranceLengthSec() <= enduranceSec) {
			shouldSave |= saveData.unlockAchievement(6);
			finishReplayRecording();
		}
		if (previousHighestChapter != saveData.highestChapter || shouldSave) {
			saveData.save();
		}

		//playerが死亡していたらBGM一時停止
		const bool isPlayerDead = avoidanceManager.getPlayer()->getIsDead();
		if (isPlayerDead) {
			pauseBgm();

			if (!wasPlayerDead) {
				saveData.addDeath(avoidanceManager.getActiveChapter());
				saveData.save();
				finishReplayRecording();
			}
		}
		wasPlayerDead = isPlayerDead;
	}

	void MainGame::updateReplayGame() {
		if (!playbackReplay.isValid()) {
			pauseBgm();
			return;
		}

		if (playbackReplay.frames.size() <= static_cast<size_t>(replayFrame)) {
			pauseBgm();
			wasPlayerDead = true;
			return;
		}

		const ReplayInputFrame inputFrame = playbackReplay.frames[replayFrame];
		avoidanceManager.setStep(playbackReplay.frameSteps[replayFrame]);
		avoidanceManager.update(inputFrame);
		++replayFrame;

		if (avoidanceManager.getPlayer()->getIsDead()
			|| playbackReplay.frames.size() <= static_cast<size_t>(replayFrame)) {
			pauseBgm();
			wasPlayerDead = true;
		}
	}

	void MainGame::debugGame() {
		canDebugChangeDifficulty = true;

		if (isTutorial) {
			return;
		}

		if (!isPracticeMode() && Global::inputDebugMuteki.down()) {
			togglePlayerMuteki();
		}

		avoidanceManager.debug();
		if (Global::inputDebugPause.down())pauseBgm();
		if (Global::inputDebugStart.down())audio.play();
	}

	void MainGame::drawGame() {
		avoidanceManager.draw();

		if (practiceLimitReached) {
			FontAsset(U"Button")(U"Practice End").draw(Vec2{ 28.0, Global::windowHeight - 44.0 }, ColorF{ 1.0, 0.82, 0.38 });
		}
		if (isPracticeMode()) {
			if (const auto player = avoidanceManager.getPlayer();
				player && player->getIsMuteki()) {
				FontAsset(U"Big")(U"★").draw(Vec2{ 28.0, Global::windowHeight - 78.0 }, ColorF{ 1.0, 0.88, 0.25 });
			}
		}
		if (isTutorial) {
			const Vec2 textPos{ 40.0, Global::windowHeight - 162.0 };
			const double lineHeight = 30.0;
			const ColorF textColor{ 0.08, 0.09, 0.12 };
			FontAsset(U"Button")(U"[←,→] move").draw(textPos, textColor);
			FontAsset(U"Button")(U"[shift] jump,double jump").draw(textPos + Vec2{ 0.0, lineHeight }, textColor);
			FontAsset(U"Button")(U"[Z] shot").draw(textPos + Vec2{ 0.0, lineHeight * 2.0 }, textColor);
			FontAsset(U"Button")(U"[R] back to main menu").draw(textPos + Vec2{ 0.0, lineHeight * 3.0 }, textColor);
		}
		if (isReplayMode()) {
			FontAsset(U"Button")(U"Replay").draw(Vec2{ 28.0, 24.0 }, ColorF{ 1.0, 0.82, 0.38 });
			FontAsset(U"Button")(U"[R] back to main menu").draw(Vec2{ 28.0, 54.0 }, ColorF{ 0.92 });
		}
		else if (canStartLastReplay()) {
			FontAsset(U"Button")(U"[Enter] replay last play").drawAt(Vec2{ Global::windowWidth * 0.5, 62.0 }, ColorF{ 1.0, 0.82, 0.38 });
			FontAsset(U"Button")(U"[R] back to main menu").drawAt(Vec2{ Global::windowWidth * 0.5, 92.0 }, ColorF{ 0.92 });
		}
	}

	void MainGame::savePeriodically(double deltaTimeSec) {
		saveTimerSec += deltaTimeSec;
		if (saveTimerSec < 1.0) {
			return;
		}

		saveTimerSec = 0.0;
		saveData.save();
	}

	void MainGame::playBgm(int32 chapter) {
		stopBgm();
		audio = AudioAsset{ U"sndHibana"};
		audio.setVolume(saveData.bgmVolume);
		SecondsF startTime = 0.0s;
		int32 startStep = 0;

		switch (chapter) {
		case 1:startStep = Global::startStep_Chapter1; break;
		case 2:startStep = Global::startStep_Chapter2; break;
		case 3:startStep = Global::startStep_Chapter3; break;
		case 4:startStep = Global::startStep_Chapter4; break;
		case 5:startStep = Global::startStep_Chapter5; break;
		case 6:startStep = Global::startStep_Chapter6; break;
		}

		startTime = SecondsF(static_cast<double>(startStep) / static_cast<double>(Global::FPS));

		audio.seekTime(startTime);
		audio.play();
	}

	void MainGame::stopBgm() {
		isTutorial = false;
		audio.stop();
		saveData.save();
	}

	void MainGame::pauseBgm() {
		audio.pause();
	}
}
