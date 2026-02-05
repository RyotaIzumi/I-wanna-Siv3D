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

		miku = std::make_shared<Miku>(Vec2{704,352});

		playBgm(1);
	}

	void MainGame::updateGame() {

		int32 newStep = static_cast<int32>(audio.posSec() * FPS);
		avoidanceManager.setStep(newStep);
		avoidanceManager.update();

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
		if (Key1.down())pauseBgm();
		if (Key2.down())audio.play();
	}

	void MainGame::drawGame() {
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

	void MainGame::playBgm(int32 chapter) {
		stopBgm();
		audio = AudioAsset{ U"sndHibana"};
		SecondsF startTime = 0.0s;
		int32 startStep = 0;

		switch (chapter) {
		case 1:startStep = 0; break;
		case 2:startStep = 840; break;
		case 3:startStep = 1320; break;
		}

		startTime = SecondsF(static_cast<double>(startStep) / static_cast<double>(FPS));

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
