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

		gameObjects.miku = std::make_shared<Miku>(Vec2{ 704,352 });
	}

	void AvoidanceManager::update() {

		//チャプターごとの更新処理
		if (step < Global::startStep_Chapter2) chapter1();
		else if(step < Global::startStep_Chapter3) chapter2();

		// ----- update関連 -----
		auto& player = gameObjects.player;
		auto& bullets = gameObjects.bullets;
		auto& cherries = gameObjects.cherries;
		auto& blocks = gameObjects.blocks;
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

		//画面外のりんごを削除
		cherries.remove_if([](auto&& cherry) {
			return cherry->isOutOfScreen;
		});

		//playerの近くのオブジェクトのみを取得して当たり判定確認
		auto near = stockNearGameObjects.query(player->getBroadRect());
		for (auto* obj : near) {
			if (obj == player.get()) continue;
			player->onCollision(*obj);
		}
		player->onCollision(*miku);
		player->updateLate();

		
		//各弾丸とブロックとの衝突
		for (auto& b : bullets) {
			auto nearObjs = stockBulletsNearGameObjects.query(b->getBroadRect());
			for (auto* obj : nearObjs) {
				b->onCollision(*obj);
			}
			b->onCollision(*miku);
		}
		
		//弾丸削除
		bullets.remove_if([](auto&& bullet) {
			return bullet->isOutOfScreen || bullet->isDelete;
		});

		miku->update();
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
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
		//ミク描画
		gameObjects.miku->draw();
		//ブロック描画
		for (auto b : gameObjects.blocks) {
			b->draw();
		}
		//kid君描画
		gameObjects.player->draw();
		//弾丸描画
		for (auto b : gameObjects.bullets) {
			b->draw();
		}
		//りんご描画
		for (auto c : gameObjects.cherries) {
			c->draw();
		}
	}

	void AvoidanceManager::setStep(int32 newStep) {
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
