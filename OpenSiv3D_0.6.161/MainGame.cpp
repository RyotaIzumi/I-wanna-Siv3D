#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
	}

	void MainGame::startGame() {
		player = std::make_shared<Player>();

		cherries.clear();
		cherries << std::make_shared<Cherry>();

		//仮ブロック配置
		blocks.clear();
		blocks << std::make_shared<Block>(Vec2(160, 544));
		blocks << std::make_shared<Block>(Vec2(160, 512));
		blocks << std::make_shared<Block>(Vec2(160, 480));
		blocks << std::make_shared<Block>(Vec2(160, 448));
		blocks << std::make_shared<Block>(Vec2(320, 448));
		blocks << std::make_shared<Block>(Vec2(320, 544));
		for (int i = 0; i < 25; i++) {
			blocks << std::make_shared<Block>(Vec2(i * 32, 576));
		}
	}

	void MainGame::updateGame() {
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

		ClearPrint();
		Print << U"付近のGameObject数 : " << near.size();

		player->updateLate();
	}

	void MainGame::debugGame() {
		
	}

	void MainGame::drawGame() {
		//背景描画
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
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
}
