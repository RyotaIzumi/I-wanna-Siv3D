#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
		saveData.load();
	}

	MainGame::~MainGame() {
		saveData.save();
	}

	void MainGame::startGame(int32 chapter) {
		lastSelectedChapter = Clamp(chapter, 1, 6);
		wasPlayerDead = false;
		shouldUpdateHighestEndurance = (lastSelectedChapter == 1);
		practiceLimitReached = false;
		practiceLimitStep = shouldUpdateHighestEndurance ? none : getPracticeLimitStep();
		saveData.updateHighestChapter(lastSelectedChapter);
		saveData.save();
		avoidanceManager.setUpObjects(lastSelectedChapter);
		playBgm(lastSelectedChapter);
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

	Optional<int32> MainGame::getPracticeLimitStep() const {
		switch (Clamp(saveData.highestChapter + 1, 1, 7)) {
		case 2: return Global::startStep_Chapter2 - 1;
		case 3: return Global::startStep_Chapter3 - 1;
		case 4: return Global::startStep_Chapter4 - 1;
		case 5: return Global::startStep_Chapter5 - 1;
		case 6: return Global::startStep_Chapter6 - 1;
		default: return none;
		}
	}

	void MainGame::updateGame() {
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
		saveData.updateHighestChapter(avoidanceManager.getActiveChapter());
		if (shouldUpdateHighestEndurance) {
			saveData.updateHighestEnduranceSec(enduranceSec);
		}
		if (previousHighestChapter != saveData.highestChapter) {
			saveData.save();
		}

		//playerが死亡していたらBGM一時停止
		const bool isPlayerDead = avoidanceManager.getPlayer()->getIsDead();
		if (isPlayerDead) {
			pauseBgm();

			if (!wasPlayerDead) {
				saveData.addDeath();
				saveData.save();
			}
		}
		wasPlayerDead = isPlayerDead;
	}

	void MainGame::debugGame() {
		avoidanceManager.debug();
		if (Global::inputDebugPause.down())pauseBgm();
		if (Global::inputDebugStart.down())audio.play();
	}

	void MainGame::drawGame() {
		avoidanceManager.draw();

		if (practiceLimitReached) {
			FontAsset(U"Button")(U"Practice End").draw(Vec2{ 28.0, Global::windowHeight - 44.0 }, ColorF{ 1.0, 0.82, 0.38 });
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
		audio.stop();
		saveData.save();
	}

	void MainGame::pauseBgm() {
		audio.pause();
	}
}
