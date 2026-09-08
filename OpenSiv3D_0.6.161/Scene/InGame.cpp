#include "Scene.h"

namespace Iwanna {
	InGame::InGame(const InitData& data) : IScene(data) {

	}

	void InGame::update() {
		auto& data = getData().game;

		data.updateGame();

		if (data.canStartLastReplay() && KeyEnter.down()) {
			data.startLastReplay();
			return;
		}

		if (Global::inputRestart.down()) {
			data.returnToStartMenu();
			changeScene(SceneType::START_MENU, 0.0s);
		}

		//debug();
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
