#include "Scene.h"

namespace Iwanna {
	StartMenu::StartMenu(const InitData& data) : IScene(data) {

	}

	void StartMenu::update() {
		auto& data = getData().game;

		if (Global::inputStart.down()) {
			data.startGame();
			changeScene(SceneType::IN_GAME, 0.0s);
		}
	}

	void StartMenu::draw() const {
		FontAsset(U"Big")(U"Press shift to start").drawAt(400,304);
	}
}
