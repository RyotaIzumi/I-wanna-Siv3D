#include "Scene.h"

namespace Iwanna {
	InGame::InGame(const InitData& data) : IScene(data) {

	}

	void InGame::update() {
		auto& data = getData().game;

		data.updateGame();

		if (Global::inputRestart.down()) {
			data.stopBgm();
			changeScene(SceneType::START_MENU, 0.0s);
		}

		debug();
	}

	void InGame::debug() const {
		auto& data = getData().game;
		data.debugGame();
	}

	void InGame::draw() const {
		auto& data = getData().game;

		data.drawGame();
	}
}
