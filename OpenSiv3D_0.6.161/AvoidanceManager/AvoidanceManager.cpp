#include "AvoidanceManager.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	AvoidanceManager::AvoidanceManager() {
		stockNearGameObjects.cellSize = 64;
		stockBulletsNearGameObjects.cellSize = 32;
	}

	void AvoidanceManager::setUpObjects(int32 chapter) {
		gameObjects.player = std::make_shared<Player>();

		gameObjects.cherries.clear();
		gameObjects.bullets.clear();
		gameObjects.bloods.clear();
		gameObjects.miku = std::make_shared<Miku>(Vec2{ 704,352 });
		Global::isInfiniteJumpMode = false;

		switch (chapter) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			//ブロック配置
			gameObjects.blocks.clear();
			createPeripheryBlocks();
			createFloorBlocks({ 3,3 });
			createFloorBlocks({ 3,6 });
			createFloorBlocks({ 3,9 });
			createFloorBlocks({ 3,12 });
			createFloorBlocks({ 3,15 });
			createFloorBlocks({ 10,3 });
			createFloorBlocks({ 10,7 });
			createFloorBlocks({ 10,11 });
			createFloorBlocks({ 10,15 });

			break;
		case 6:
			Global::isInfiniteJumpMode = true;
			break;
		}

		// 一部変数の初期化
		isGenerateBloods = false;
		previousStep = -1;
	}

	void AvoidanceManager::update() {

		//チャプターごとの更新処理
		if (step < Global::startStep_Chapter2) chapter1();
		else if(step < Global::startStep_Chapter3) chapter2();
		else if (step < Global::startStep_Chapter4) chapter3();
		else if (step < Global::startStep_Chapter5) chapter4();
		else if (step < Global::startStep_Chapter6) chapter5();
		else chapter6();

		// ----- update関連 -----
		auto& player = gameObjects.player;
		auto& bullets = gameObjects.bullets;
		auto& cherries = gameObjects.cherries;
		auto& blocks = gameObjects.blocks;
		auto& bloods = gameObjects.bloods;
		auto& miku = gameObjects.miku;

		player->update();

		// 弾丸の生成
		if (player->getIsGenerateBullet()) {
			if (bullets.size() < bulletMaxNum) {
				bullets << std::make_shared<Bullet>(player->pos, player->getDirection() == Global::Direction::RIGHT ? bulletSpeed : -bulletSpeed);
				AudioAsset(Sound::SHOOT).playOneShot();
			}
			player->setIsGenerateBullet(false);
		}

		// 血しぶきの生成
		if (player->getIsDead() && !isGenerateBloods) {
			double circleNum = 2;
			double deltaD = 360 / bloodNum;
			for (int32 count = 0; count < circleNum; count++) {
				for (int32 i = 0; i < bloodNum / circleNum; i++) {
					bloods << std::make_shared<Blood>(player->pos, i * deltaD);
				}
			}
			isGenerateBloods = true;
		}
		for (auto& bl : bloods) {
			bl->update();
			stockNearGameObjects.add(bl.get());
		}

		//毎フレームGameObjectをspatialGridに登録
		stockNearGameObjects.clear();
		stockBulletsNearGameObjects.clear();

		stockNearGameObjects.add(player.get());
		for (auto& b : blocks) {
			stockNearGameObjects.add(b.get());
			stockBulletsNearGameObjects.add(b.get());
		}
		for (auto& b : bullets) {
			b->update();
		}
		for (auto& c : cherries) {
			c->update();
			stockNearGameObjects.add(c.get());
		}

		//playerの近くのオブジェクトのみを取得して当たり判定確認
		auto near = stockNearGameObjects.query(player->getBroadRect());
		for (auto* obj : near) {
			if (obj == player.get()) continue;
			player->onCollision(*obj);
		}
		player->onCollision(*miku);
		player->updateLate();

		//血のブロックに対する衝突
		if (!bloods.isEmpty()) {
			for (auto& b : bloods) {
				auto nearObjs = stockNearGameObjects.query(b->getBroadRect());
				for (auto* obj : nearObjs) {
					b->onCollision(*obj);
				}
			}
		}
		
		//各弾丸とブロックとの衝突
		for (auto& b : bullets) {
			auto nearObjs = stockBulletsNearGameObjects.query(b->getBroadRect());
			for (auto* obj : nearObjs) {
				b->onCollision(*obj);
			}
			b->onCollision(*miku);
		}

		miku->update();
		
		//弾丸削除
		bullets.remove_if([](auto&& bullet) {
			return bullet->isOutOfScreen || bullet->isDelete;
		});
		//画面外のりんごを削除
		cherries.remove_if([](auto&& cherry) {
			return cherry->isOutOfScreen;
		});
	}

	void AvoidanceManager::debug() {
		auto& player = gameObjects.player;

		if (Global::inputDebugMuteki.down()) {
			player->setIsMuteki(!player->getIsMuteki());
		}

		ClearPrint();
		Print << U" Avoidance Step : " << step;
		Print << U" Cherries Num : " << gameObjects.cherries.size();
		Print << U" Player Pos : " << player->pos;
		Print << U" Player Muteki : " << player->getIsMuteki();
		Print << U" Bullets Num : " << gameObjects.bullets.size();
	}

	void AvoidanceManager::draw() const {
		//背景描画
		Rect(0, 0, 800, 608).draw(ColorF(0.8, 1.0));

		Array<std::shared_ptr<GameObject>> drawList;

		//drawListに突っ込む
		drawList << gameObjects.miku;
		drawList << gameObjects.player;
		for (auto b : gameObjects.blocks)drawList << b;
		for (auto& c : gameObjects.cherries) drawList << c;
		for (auto& b : gameObjects.bloods) drawList << b;
		for (auto& b : gameObjects.bullets) drawList << b;

		// ソート
		drawList.sort_by([](const auto& a, const auto& b) {
			return a->depth < b->depth;
		});

		// 描画
		for (auto& obj : drawList) obj->draw();
	}

	void AvoidanceManager::setStep(int32 newStep) {
		if (previousStep < 0 || newStep < step) {
			previousStep = newStep - 1;
		}
		else {
			previousStep = step;
		}
		step = newStep;
	}

	std::shared_ptr<Player> AvoidanceManager::getPlayer() {
		return gameObjects.player;
	}

	Array<std::shared_ptr<Cherry>> AvoidanceManager::getCherries() {
		return gameObjects.cherries;
	}

	Array<std::shared_ptr<Block>> AvoidanceManager::getBlocks() {
		return gameObjects.blocks;
	}

	std::shared_ptr<Miku> AvoidanceManager::getMiku() {
		return gameObjects.miku;
	}

	//りんご生成と管理配列への追加
	void AvoidanceManager::createCherry(std::shared_ptr<Cherry> cherry) {
		gameObjects.cherries << cherry;
	}

	//外周のブロック配置
	void AvoidanceManager::createPeripheryBlocks() {
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(0, 0));
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(24, 0));
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(0, 18));
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(24, 18));
		for (int i = 1; i < 18; i++) {
			gameObjects.blocks << std::make_shared<Block>(U"sprWall", Vec2(0, i));
			gameObjects.blocks << std::make_shared<Block>(U"sprWall", Vec2(24, i));
		}
		for (int i = 1; i < 24; i++) {
			gameObjects.blocks << std::make_shared<Block>(U"sprFloor", Vec2(i, 0));
			gameObjects.blocks << std::make_shared<Block>(U"sprFloor", Vec2(i, 18));
		}
	}

	//5マス分の床ブロックを作成
	void AvoidanceManager::createFloorBlocks(Vec2 basePos) {
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x, basePos.y));
		for (int i = 1; i <= 3; i++) {
			gameObjects.blocks << std::make_shared<Block>(U"sprFloor", Vec2(basePos.x + i, basePos.y));
		}
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x + 4, basePos.y));
	}
}
