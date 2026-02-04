#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
	}

	void MainGame::startGame() {
		player = std::make_shared<Player>();

		cherries.clear();
		cherries << std::make_shared<Cherry>();

		//ブロック配置
		blocks.clear();
		createPeripheryBlocks();
		createFloorBlocks({3,3});
		createFloorBlocks({3,6});
		createFloorBlocks({3,9});
		createFloorBlocks({3,12});
		createFloorBlocks({3,15});
		createFloorBlocks({10,3});
		createFloorBlocks({10,7});
		createFloorBlocks({10,11});
		createFloorBlocks({10,15});
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

	//外周のブロック配置
	void MainGame::createPeripheryBlocks() {
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(0, 0));
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(24, 0));
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(0, 18));
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(24, 18));
		for (int i = 1; i < 18; i++) {
			blocks << std::make_shared<Block>(U"sprWall", Vec2(0, i));
			blocks << std::make_shared<Block>(U"sprWall", Vec2(24, i));
		}
		for (int i = 1; i < 24; i++) {
			blocks << std::make_shared<Block>(U"sprFloor", Vec2(i, 0));
			blocks << std::make_shared<Block>(U"sprFloor", Vec2(i, 18));
		}
	}

	//5マス分の床ブロックを作成
	void MainGame::createFloorBlocks(Vec2 basePos) {
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x, basePos.y));
		for (int i = 1; i <= 3; i++) {
			blocks << std::make_shared<Block>(U"sprFloor", Vec2(basePos.x + i, basePos.y));
		}
		blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x + 4, basePos.y));
	}
}
