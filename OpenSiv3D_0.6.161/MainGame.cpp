#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
	}

	void MainGame::startGame() {
		playBgm(3);
	}

	void MainGame::updateGame() {
		int32 newStep = static_cast<int32>(audio.posSec() * Global::FPS);
		avoidanceManager.setStep(newStep);
		avoidanceManager.update();

		auto player = avoidanceManager.getPlayer();
		auto blocks = avoidanceManager.getBlocks();
		auto cherries = avoidanceManager.getCherries();
		auto miku = avoidanceManager.getMiku();

		player->update();

		//毎フレームGameObjectをspatialGridに登録
		stockNearGameObjects.clear();
		stockNearGameObjects.add(player.get());
		for (auto& b : blocks) stockNearGameObjects.add(b.get());
		for (auto& c : cherries) stockNearGameObjects.add(c.get());

		//playerの近くのオブジェクトのみを取得して当たり判定確認
		auto near = stockNearGameObjects.query(player->getBroadRect());

		for (auto* obj : near) {
			if (obj == player.get()) continue;
			player->onCollision(*obj);
		}

		player->onCollision(*miku);

		if (player->getIsDead()) pauseBgm();

		player->updateLate();

		miku->update();
	}

	void MainGame::debugGame() {
		if (Global::inputDebugPause.down())pauseBgm();
		if (Global::inputDebugStart.down())audio.play();
	}

	void MainGame::drawGame() {
		auto player = avoidanceManager.getPlayer();
		auto blocks = avoidanceManager.getBlocks();
		auto cherries = avoidanceManager.getCherries();
		auto miku = avoidanceManager.getMiku();

		//背景描画
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
		//ミク描画
		miku->draw();
		//ブロック描画
		for (auto b : blocks) {
			b->draw();
		}
		//kid君描画
		player->draw();
		//さくらんぼ描画
		for (auto c : cherries) {
			c->draw();
		}
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
