#include "StageManager.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	StageManager::StageManager() {
		stockNearGameObjects.cellSize = 64;
		stockBulletsNearGameObjects.cellSize = 32;
	}

	void StageManager::setUpObjects(int32 chapter) {
		gameObjects.player = std::make_shared<Player>();

		// 既存のオブジェクトを抹消して初期化
		gameObjects.cherries.clear();
		gameObjects.blocks.clear();
		gameObjects.spikes.clear();
		gameObjects.triggers.clear();

		loadGameObjects(U"test");
	}

	void StageManager::loadGameObjects(String fileName) {
		//ステージデータの読み込みとオブジェクト生成
		CSV csv{ U"MapData/" + fileName + U".csv"};

		if (!csv)
		{
			throw Error{ U"次のCSVが読み込めません : " + fileName + U".csv"};
		}

		for (size_t y = 0; y < csv.rows(); ++y)
		{
			for (size_t x = 0; x < csv.columns(y); ++x)
			{
				int value = csv.get<int>(y, x);

				// 0 は空白として無視
				if (value == 0)
					continue;

				Vec2 pos{
					x,
					y
				};

				// value に応じて配置
				switch (value) {
				case 1: gameObjects.blocks << std::make_shared<Block>(U"sprBlock", pos); break;
				case 6: gameObjects.blocks << std::make_shared<Block>(U"sprFloor", pos); break;
				case 7: gameObjects.blocks << std::make_shared<Block>(U"sprWall", pos); break;
				case 21: gameObjects.spikes << std::make_shared<Spike>(pos, 0); break;
				case 22: gameObjects.spikes << std::make_shared<Spike>(pos, 1); break;
				case 23: gameObjects.spikes << std::make_shared<Spike>(pos, 2); break;
				case 24: gameObjects.spikes << std::make_shared<Spike>(pos, 3); break;
				}
			}
		}

		// 次に同名のJsonファイルからギミックデータを読み込む

		const String jsonPath = U"MapData/" + fileName + U".json";
		const auto& jsonArray = JSON::Load(jsonPath);

		// ロードに失敗した場合はエラーを投げる
		if (not jsonArray) {
			throw Error{ U"対応するJSONファイルをロードできませんでした : " + jsonPath };
		}

		// ステージ情報をパース
		for (const auto& stage : jsonArray.arrayView()) {
			// プレイヤーの初期位置を取得し反映
			Vec2 startPlayerPos = parsePos(stage[U"startPlayerPos"]);
			startPlayerPos *= oneTileSize;
			gameObjects.player->pos = startPlayerPos;

			String gimmikName;
			Vec2 gimmikPos;
			double gimmikValue1;
			double gimmikValue2;
			double gimmikValue3;

			if (stage.contains(U"Gimmiks")) {
				for (const auto& gimmik : stage[U"Gimmiks"].arrayView()) {
					gimmikName = gimmik[U"gimmikName"].getString();
					gimmikPos = parsePos(gimmik[U"gimmikPos"]);
					gimmikValue1 = gimmik[U"id"].get<double>();
					gimmikValue2 = gimmik[U"direction"].get<double>();
					gimmikValue3 = gimmik[U"speed"].get<double>();

					if (gimmikName == U"罠針_上") gameObjects.spikes << std::make_shared<SpikeTrap>(gimmikPos, 0, static_cast<int32>(gimmikValue1), gimmikValue2, gimmikValue3);
					if (gimmikName == U"罠針_左") gameObjects.spikes << std::make_shared<SpikeTrap>(gimmikPos, 1, static_cast<int32>(gimmikValue1), gimmikValue2, gimmikValue3);
					if (gimmikName == U"罠針_下") gameObjects.spikes << std::make_shared<SpikeTrap>(gimmikPos, 2, static_cast<int32>(gimmikValue1), gimmikValue2, gimmikValue3);
					if (gimmikName == U"罠針_右") gameObjects.spikes << std::make_shared<SpikeTrap>(gimmikPos, 3, static_cast<int32>(gimmikValue1), gimmikValue2, gimmikValue3);
					if (gimmikName == U"罠トリガー") gameObjects.triggers << std::make_shared<Trigger>(gimmikPos, static_cast<int32>(gimmikValue1), 1.0, 1.0);
				}
			}
			// 敵の情報がない場合のエラーハンドリング
			else {
				throw Error{ U"ギミックの情報がありません" };
			}
		}
	}

	Vec2 StageManager::parsePos(const JSON& json) {
		return Vec2{ json[0].get<int32>() / oneTileSize, json[1].get<int32>() / oneTileSize };
	}

	void StageManager::update() {

		// ----- update関連 -----
		auto& player = gameObjects.player;
		auto& bullets = gameObjects.bullets;
		auto& cherries = gameObjects.cherries;
		auto& blocks = gameObjects.blocks;
		auto& spikes = gameObjects.spikes;
		auto& triggers = gameObjects.triggers;

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

		// トリガーの更新と、最新の起動トリガーIDの取得
		int32 latestActivatedTriggerID = -1;
		for (auto& t : triggers) {
			if (t->getIsActivated()) {
				latestActivatedTriggerID = t->getTrapID();
			}
			stockNearGameObjects.add(t.get());
		}
		// 針の更新と、起動しているトリガーIDの反映
		for (auto& s : spikes) {
			s->trapUpdate(latestActivatedTriggerID);
			stockNearGameObjects.add(s.get());
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
		player->updateLate();


		//各弾丸とブロックとの衝突
		for (auto& b : bullets) {
			auto nearObjs = stockBulletsNearGameObjects.query(b->getBroadRect());
			for (auto* obj : nearObjs) {
				b->onCollision(*obj);
			}
		}

		//弾丸削除
		bullets.remove_if([](auto&& bullet) {
			return bullet->isOutOfScreen || bullet->isDelete;
		});
	}

	void StageManager::debug() {
		auto& player = gameObjects.player;

		if (Global::inputDebugMuteki.down()) {
			player->setIsMuteki(!player->getIsMuteki());
		}

		ClearPrint();
		Print << U" Stage Step : " << step;
		Print << U" Cherries Num : " << gameObjects.cherries.size();
		Print << U" Player Pos : " << player->pos;
		Print << U" Player Muteki : " << player->getIsMuteki();
		Print << U" Bullets Num : " << gameObjects.bullets.size();
	}

	void StageManager::draw() const {
		//背景描画
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
		//ブロック描画
		for (auto b : gameObjects.blocks) {
			b->draw();
		}
		//針描画
		for (auto s : gameObjects.spikes) {
			s->draw();
		}
		//トリガー描画
		for (auto t : gameObjects.triggers) {
			t->draw();
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

	void StageManager::setStep(int32 newStep) {
		step = newStep;
	}

	std::shared_ptr<Player> StageManager::getPlayer() {
		return gameObjects.player;
	}

	Array<std::shared_ptr<Cherry>> StageManager::getCherries() {
		return gameObjects.cherries;
	}

	Array<std::shared_ptr<Block>> StageManager::getBlocks() {
		return gameObjects.blocks;
	}

	//りんご生成と管理配列への追加
	void StageManager::createCherry(std::shared_ptr<Cherry> cherry) {
		gameObjects.cherries << cherry;
	}

	//外周のブロック配置
	void StageManager::createPeripheryBlocks() {
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
	void StageManager::createFloorBlocks(Vec2 basePos) {
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x, basePos.y));
		for (int i = 1; i <= 3; i++) {
			gameObjects.blocks << std::make_shared<Block>(U"sprFloor", Vec2(basePos.x + i, basePos.y));
		}
		gameObjects.blocks << std::make_shared<Block>(U"sprBlock", Vec2(basePos.x + 4, basePos.y));
	}
}
