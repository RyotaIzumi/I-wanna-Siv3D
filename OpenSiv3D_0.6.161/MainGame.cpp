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
		saveData.save();
	}

	void MainGame::startGame(int32 chapter) {
		if (!canStartAvoidance()) {
			return;
		}

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
		avoidanceManager.setUpObjects(lastSelectedChapter);
		playBgm(lastSelectedChapter);
	}

	void MainGame::startTutorial() {
		isTutorial = true;
		wasPlayerDead = false;
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

	void MainGame::updateGame() {
		if (isTutorial) {
			avoidanceManager.updateTutorial();
			return;
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

		avoidanceManager.setStep(newStep);
		avoidanceManager.update();
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
			}
		}
		wasPlayerDead = isPlayerDead;
	}

	void MainGame::debugGame() {
		canDebugChangeDifficulty = true;

		if (isTutorial) {
			return;
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
		if (isTutorial) {
			const Vec2 textPos{ 40.0, Global::windowHeight - 162.0 };
			const double lineHeight = 30.0;
			const ColorF textColor{ 0.08, 0.09, 0.12 };
			FontAsset(U"Button")(U"[←,→] move").draw(textPos, textColor);
			FontAsset(U"Button")(U"[shift] jump,double jump").draw(textPos + Vec2{ 0.0, lineHeight }, textColor);
			FontAsset(U"Button")(U"[Z] shot").draw(textPos + Vec2{ 0.0, lineHeight * 2.0 }, textColor);
			FontAsset(U"Button")(U"[R] back to main menu").draw(textPos + Vec2{ 0.0, lineHeight * 3.0 }, textColor);
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
