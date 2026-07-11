#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
	}

	void MainGame::startGame(int32 chapter) {
		avoidanceManager.setUpObjects(chapter);
		playBgm(chapter);
	}

	void MainGame::updateGame() {
		int32 newStep = static_cast<int32>(audio.posSec() * Global::FPS);
		avoidanceManager.setStep(newStep);
		avoidanceManager.update();

		//playerが死亡していたらBGM一時停止
		if (avoidanceManager.getPlayer()->getIsDead()) pauseBgm();
	}

	void MainGame::debugGame() {
		avoidanceManager.debug();
		if (Global::inputDebugPause.down())pauseBgm();
		if (Global::inputDebugStart.down())audio.play();
	}

	void MainGame::drawGame() {
		avoidanceManager.draw();
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
	}

	void MainGame::pauseBgm() {
		audio.pause();
	}
}
