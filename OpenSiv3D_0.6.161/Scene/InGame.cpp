#include "Scene.h"

namespace Iwanna {
	InGame::InGame(const InitData& data) : IScene(data) {

	}

	void InGame::update() {
		auto& data = getData().game;

		data.updateGame();
		debug();
	}

	void InGame::debug() const {
		
	}

	void InGame::draw() const {
		auto& data = getData().game;

		data.drawGame();
	}
}
