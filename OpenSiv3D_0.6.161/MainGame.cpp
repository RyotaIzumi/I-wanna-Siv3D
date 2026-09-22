#include "MainGame.h"

#include "Audio/AudioAsset.h"
namespace Iwanna {
	MainGame::MainGame() {
		saveData.load();
		Global::bgmVolume = saveData.bgmVolume;
		Global::seVolume = saveData.seVolume;
		Global::difficulty = saveData.difficulty;
	}

	MainGame::~MainGame() {
		replayManager.finishRecording();
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
		if (lastSelectedChapter == 1) {
			replayManager.beginRecording(lastSelectedChapter, getChapterStartStep(lastSelectedChapter), Global::difficulty);
		}
		else {
			replayManager.cancelRecording();
			Reseed(RandomUint64());
		}
		avoidanceManager.setUpObjects(lastSelectedChapter);
		playBgm(lastSelectedChapter);
	}

	void MainGame::startTutorial() {
		playMode = PlayMode::Normal;
		isTutorial = true;
		wasPlayerDead = false;
		replayManager.cancelRecording();
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
		return replayManager.getReplayCount();
	}

	const ReplayData* MainGame::getReplay(size_t index) const {
		return replayManager.getReplay(index);
	}

	bool MainGame::canStartReplay(size_t index, int32 startChapter) const {
		const ReplayData* replay = getReplay(index);
		const int32 startStep = getChapterStartStep(startChapter);
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& replay
			&& replay->isValid()
			&& startChapter == Clamp(startChapter, 1, 6)
			&& startStep <= replay->frameSteps.back();
	}

	void MainGame::startReplay(size_t index, int32 startChapter) {
		if (!canStartReplay(index, startChapter)) {
			return;
		}

		startReplayData(*replayManager.getReplay(index), startChapter);
	}

	size_t MainGame::getFavoriteReplayCount() const {
		return replayManager.getFavoriteReplayCount();
	}

	const ReplayData* MainGame::getFavoriteReplay(size_t index) const {
		return replayManager.getFavoriteReplay(index);
	}

	bool MainGame::isReplayFavorite(size_t index) const {
		return replayManager.isReplayFavorite(index);
	}

	bool MainGame::canAddReplayToFavorites(size_t index) const {
		return replayManager.canAddReplayToFavorites(index);
	}

	void MainGame::addReplayToFavorites(size_t index) {
		replayManager.addReplayToFavorites(index);
	}

	void MainGame::removeFavoriteReplay(size_t index) {
		replayManager.removeFavoriteReplay(index);
	}

	bool MainGame::canStartFavoriteReplay(size_t index, int32 startChapter) const {
		const ReplayData* replay = getFavoriteReplay(index);
		const int32 startStep = getChapterStartStep(startChapter);
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& replay
			&& replay->isValid()
			&& startChapter == Clamp(startChapter, 1, 6)
			&& startStep <= replay->frameSteps.back();
	}

	void MainGame::startFavoriteReplay(size_t index, int32 startChapter) {
		if (!canStartFavoriteReplay(index, startChapter)) {
			return;
		}

		startReplayData(*replayManager.getFavoriteReplay(index), startChapter);
	}

	void MainGame::startReplayData(const ReplayData& replay, int32 startChapter) {

		stopBgm();
		playMode = PlayMode::Replay;
		isTutorial = false;
		replayManager.cancelRecording();
		wasPlayerDead = false;
		practiceLimitReached = false;
		practiceLimitStep = none;
		replayManager.beginPlayback(replay);
		Global::difficulty = replay.difficulty;
		avoidanceManager.setUpObjects(replay.chapter);
		const int32 startStep = getChapterStartStep(startChapter);
		while (replayManager.hasPlaybackFrame() && replayManager.getPlaybackStep() < startStep) {
			avoidanceManager.setStep(replayManager.getPlaybackStep());
			avoidanceManager.update(replayManager.getPlaybackInput());
			replayManager.advancePlaybackFrame();
		}
		audio = AudioAsset{ U"sndHibana" };
		audio.setVolume(saveData.bgmVolume);
		audio.setSpeed(1.0);
		audio.seekTime(SecondsF(static_cast<double>(startStep) / static_cast<double>(Global::FPS)));
		audio.play();
	}

	bool MainGame::canStartLastReplay() const {
		return playMode == PlayMode::Normal
			&& !isTutorial
			&& wasPlayerDead
			&& replayManager.getReplayCount() != 0
			&& replayManager.getReplay(0)->isValid();
	}

	void MainGame::startLastReplay() {
		if (!canStartLastReplay()) {
			return;
		}

		startReplay(0);
	}

	void MainGame::returnToStartMenu() {
		const bool shouldKeepLastReplaySelectable = isReplayMode() && replayManager.getReplayCount() != 0;
		replayManager.finishRecording();
		stopBgm();
		playMode = PlayMode::Normal;
		isTutorial = false;
		replayManager.cancelRecording();
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
		if (wasAliveAtFrameStart) {
			replayManager.recordFrame(inputFrame, newStep);
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
			replayManager.finishRecording();
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
				const String screenshotPath = replayManager.finishRecordingWithScreenshot();
				if (!screenshotPath.isEmpty()) {
					ScreenCapture::SaveCurrentFrame(screenshotPath);
				}
			}
		}
		wasPlayerDead = isPlayerDead;
	}

	void MainGame::updateReplayGame() {
		if (!replayManager.hasPlaybackFrame()) {
			pauseBgm();
			wasPlayerDead = true;
			return;
		}

		const bool isSlowPlayback = KeyDown.pressed();
		audio.setSpeed(isSlowPlayback ? 0.5 : 1.0);
		if (!replayManager.shouldAdvancePlayback(isSlowPlayback)) {
			return;
		}

		avoidanceManager.setStep(replayManager.getPlaybackStep());
		avoidanceManager.update(replayManager.getPlaybackInput());
		replayManager.advancePlaybackFrame();

		if (avoidanceManager.getPlayer()->getIsDead()
			|| !replayManager.hasPlaybackFrame()) {
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
			FontAsset(U"Button")(U"[Down] hold for 0.5x").draw(Vec2{ 28.0, 84.0 }, ColorF{ 0.92 });
			if (KeyDown.pressed()) {
				FontAsset(U"Button")(U"0.5x").draw(Vec2{ 28.0, 114.0 }, ColorF{ 1.0, 0.82, 0.38 });
			}
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
