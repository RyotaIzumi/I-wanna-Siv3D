#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
		player = std::make_shared<Player>();

		//仮ブロック配置
		blocks << std::make_shared<Block>(Vec2(160, 544));
		blocks << std::make_shared<Block>(Vec2(160, 512));
		blocks << std::make_shared<Block>(Vec2(160, 480));
		blocks << std::make_shared<Block>(Vec2(160, 448));
		blocks << std::make_shared<Block>(Vec2(320, 448));
		for(int i = 0;i < 25;i++){
			blocks << std::make_shared<Block>(Vec2(i * 32, 576));
		}
	}

	void MainGame::updateGame() {
		player->update();
		for (auto b : blocks) {
			player->onCollision(*b);
			b->update();
		}
		player->updateLate();
	}

	void MainGame::debugGame() {

	}

	void MainGame::drawGame() {
		//背景描画
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
		//kid君描画
		player->draw();
		//ブロック描画
		for (auto b : blocks) {
			player->onCollision(*b);
			b->draw();
		}
	}
}
