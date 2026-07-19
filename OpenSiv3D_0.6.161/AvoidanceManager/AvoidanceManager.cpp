#include "AvoidanceManager.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	AvoidanceManager::AvoidanceManager() {
		stockNearGameObjects.cellSize = 64;
		stockBulletsNearGameObjects.cellSize = 32;

		// 実体は必要になった分だけ生成し、タイトル画面での大量初期化を避ける。
		constexpr size_t cherryPoolReserveCapacity = 1024;
		inactiveCherries.reserve(cherryPoolReserveCapacity);
	}

	void AvoidanceManager::setUpObjects(int32 chapter) {
		const bool shouldStartChapterTransitionFade = (activeChapter != 0 && activeChapter != chapter);

		gameObjects.player = std::make_shared<Player>();

		stockNearGameObjects.clear();
		stockBulletsNearGameObjects.clear();
		recycleAllCherries();
		gameObjects.bullets.clear();
		gameObjects.bloods.clear();
		gameObjects.blocks.clear();
		applyChapterSettings(createChapterSettings(chapter));
		markDrawListDirty();

		// 一部変数の初期化
		activeChapter = chapter;
		isGenerateBloods = false;
		previousStep = -1;
		screenShakeActive = false;
		screenShakeStopwatch.reset();

		if (shouldStartChapterTransitionFade) {
			requestChapterTransitionFade(chapterTransitionFadeDurationStep);
		}
	}

	int32 AvoidanceManager::getChapterFromStep(int32 targetStep) const {
		if (targetStep < Global::startStep_Chapter2) return 1;
		if (targetStep < Global::startStep_Chapter3) return 2;
		if (targetStep < Global::startStep_Chapter4) return 3;
		if (targetStep < Global::startStep_Chapter5) return 4;
		if (targetStep < Global::startStep_Chapter6) return 5;
		return 6;
	}

	ChapterSettings AvoidanceManager::createChapterSettings(int32 chapter) const {
		ChapterSettings settings;

		switch (chapter) {
		case 1:
			settings.playerPos = Vec2{ 400,300 };
			settings.mikuPos = Vec2{ 1800,352 };
			settings.backgroundColor = ColorF(0.0, 1.0);
			settings.isInfiniteJumpMode = true;
			break;
		case 2:
			settings.playerPos = Vec2{ 400,300 };

			addPeripheryBlockSettings(settings.blocks);
			addFloorBlockSettings(settings.blocks, Vec2{ 3,3 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,6 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,9 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,12 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,15 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,3 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,7 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,11 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,15 });
			break;
		case 3:
			settings.playerPos = Vec2{ 400,300 };
			settings.backgroundColor = ColorF(0.5, 1.0);
			settings.isInfiniteJumpMode = true;
			break;
		case 4:
		case 5:
			addPeripheryBlockSettings(settings.blocks);
			addFloorBlockSettings(settings.blocks, Vec2{ 3,3 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,6 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,9 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,12 });
			addFloorBlockSettings(settings.blocks, Vec2{ 3,15 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,3 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,7 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,11 });
			addFloorBlockSettings(settings.blocks, Vec2{ 10,15 });
			break;
		case 6:
			settings.isInfiniteJumpMode = true;
			break;
		default:
			break;
		}

		return settings;
	}

	void AvoidanceManager::applyChapterSettings(const ChapterSettings& settings) {
		backgroundColor = settings.backgroundColor;
		Global::isInfiniteJumpMode = settings.isInfiniteJumpMode;

		gameObjects.player->pos = settings.playerPos;
		gameObjects.player->hitBox->setPos(settings.playerPos);
		gameObjects.player->setDepth(settings.playerDepth);
		gameObjects.miku = std::make_shared<Miku>(settings.mikuPos);
		gameObjects.miku->setDepth(settings.mikuDepth);

		for (const auto& blockSetting : settings.blocks) {
			auto block = std::make_shared<Block>(blockSetting.textureName, blockSetting.gridPos);
			block->setHasCollide(blockSetting.hasCollide);
			block->setDepth(blockSetting.depth);
			gameObjects.blocks << block;
		}
	}

	void AvoidanceManager::addPeripheryBlockSettings(Array<BlockPlacement>& blocks) const {
		blocks << BlockPlacement{ U"sprBlock", Vec2{ 0,0 } };
		blocks << BlockPlacement{ U"sprBlock", Vec2{ 24,0 } };
		blocks << BlockPlacement{ U"sprBlock", Vec2{ 0,18 } };
		blocks << BlockPlacement{ U"sprBlock", Vec2{ 24,18 } };

		for (int32 i = 1; i < 18; ++i) {
			blocks << BlockPlacement{ U"sprWall", Vec2{ 0,i } };
			blocks << BlockPlacement{ U"sprWall", Vec2{ 24,i } };
		}

		for (int32 i = 1; i < 24; ++i) {
			blocks << BlockPlacement{ U"sprFloor", Vec2{ i,0 } };
			blocks << BlockPlacement{ U"sprFloor", Vec2{ i,18 } };
		}
	}

	void AvoidanceManager::addFloorBlockSettings(Array<BlockPlacement>& blocks, Vec2 basePos) const {
		blocks << BlockPlacement{ U"sprBlock", Vec2{ basePos.x,basePos.y } };

		for (int32 i = 1; i <= 3; ++i) {
			blocks << BlockPlacement{ U"sprFloor", Vec2{ basePos.x + i,basePos.y } };
		}

		blocks << BlockPlacement{ U"sprBlock", Vec2{ basePos.x + 4,basePos.y } };
	}

	void AvoidanceManager::update() {
		const int32 chapter = getChapterFromStep(step);
		if (chapter != activeChapter) {
			setUpObjects(chapter);
		}

		//チャプターごとの更新処理
		switch (chapter) {
		case 1: chapter1(); break;
		case 2: chapter2(); break;
		case 3: chapter3(); break;
		case 4: chapter4(); break;
		case 5: chapter5(); break;
		case 6: chapter6(); break;
		default: break;
		}

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
				markDrawListDirty();
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
			markDrawListDirty();
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
		}

		//playerの近くのオブジェクトのみを取得して当たり判定確認
		auto near = stockNearGameObjects.query(player->getBroadRect());
		for (auto* obj : near) {
			if (obj == player.get()) continue;
			player->onCollision(*obj);
		}

		// Cherry は数が多いため spatial grid へ登録しない。
		// プレイヤーの矩形と重なる候補だけ、正確な当たり判定へ進める。
		const RectF playerBroadRect = player->getBroadRect();
		for (const auto& cherry : cherries) {
			if (cherry->canPlayerKill
				&& cherry->getBroadRect().intersects(playerBroadRect)) {
				player->onCollision(*cherry);
			}
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
		const size_t bulletCountBeforeRemove = bullets.size();
		bullets.remove_if([](auto&& bullet) {
			return bullet->isOutOfScreen || bullet->isDelete;
		});
		if (bullets.size() != bulletCountBeforeRemove) {
			markDrawListDirty();
		}
		//画面外のりんごを削除
		const size_t cherryCountBeforeRemove = cherries.size();
		cherries.remove_if([this](const auto& cherry) {
			if (!(cherry->isOutOfScreen || cherry->isDelete)) {
				return false;
			}

			recycleCherry(cherry);
			return true;
		});
		if (cherries.size() != cherryCountBeforeRemove) {
			markDrawListDirty();
		}
	}

	void AvoidanceManager::debug() {
		auto& player = gameObjects.player;

		if (Global::inputDebugMuteki.down()) {
			player->setIsMuteki(!player->getIsMuteki());
		}

		ClearPrint();
		Print << U" Avoidance Step : " << step;
		Print << U" Chapter : " << activeChapter;
		
		Print << U" Cherries Num : " << gameObjects.cherries.size();

		/*
		Print << U" Player Pos : " << player->pos;
		Print << U" Player Muteki : " << player->getIsMuteki();
		Print << U" Bullets Num : " << gameObjects.bullets.size();
		Print << U" Depth Player/Miku : " << player->getDepth() << U" / " << gameObjects.miku->getDepth();
		if (!gameObjects.blocks.isEmpty()) Print << U" Depth Block : " << gameObjects.blocks.front()->getDepth();
		if (!gameObjects.cherries.isEmpty()) Print << U" Depth Cherry : " << gameObjects.cherries.front()->getDepth();
		if (!gameObjects.bullets.isEmpty()) Print << U" Depth Bullet : " << gameObjects.bullets.front()->getDepth();
		if (!gameObjects.bloods.isEmpty()) Print << U" Depth Blood : " << gameObjects.bloods.front()->getDepth();
		*/
	}

	void AvoidanceManager::draw() const {
		const double screenShakeY = getScreenShakeOffset();
		{
			const Transformer2D screenShakeTransformer{ Mat3x2::Translate(0.0, screenShakeY) };
			// Cherry ごとではなく、描画パス全体で一度だけ設定する。
			const ScopedRenderStates2D nearestSampler{ SamplerState::ClampNearest };

			//背景描画
			Rect(-16, -16, 832, 640).draw(backgroundColor);

			rebuildDrawListIfNeeded();
			const RectF cherryVisibleArea{ -32.0, -32.0,
				Global::windowWidth + 64.0, Global::windowHeight + 64.0 };
			const double chapter2SatBarrageFrontDepth = DrawDepth::Player + 10.0;

			for (auto* obj : sortedDrawList) {
				if (obj->getDepth() < chapter2SatBarrageFrontDepth) {
					if (obj->type == ObjectType::Cherry
						&& !obj->getBroadRect().intersects(cherryVisibleArea)) {
						continue;
					}
					obj->draw();
				}
			}

			drawChapter2SatBarrageMasks();
			drawChapter1OpeningFade();
			drawChapter1SniperSights();

			for (auto* obj : sortedDrawList) {
				if (chapter2SatBarrageFrontDepth <= obj->getDepth()) {
					if (obj->type == ObjectType::Cherry
						&& !obj->getBroadRect().intersects(cherryVisibleArea)) {
						continue;
					}
					obj->draw();
				}
			}

			drawChapter2SniperSight();
			drawChapter1EndingFade();
		}

		const double fadeAlpha = getChapterTransitionFadeAlpha();
		if (0.0 < fadeAlpha) {
			ColorF fadeColor = chapterTransitionFadeColor;
			fadeColor.a *= fadeAlpha;
			Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(fadeColor);
		}
	}

	double AvoidanceManager::getScreenShakeOffset() const {
		if (!screenShakeActive) {
			return 0.0;
		}

		const double age = screenShakeStopwatch.sF() * 50.0;
		if (screenShakeDurationStep < age) {
			return 0.0;
		}

		const double t = age / static_cast<double>(Max(screenShakeDurationStep, 1));
		const double decay = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
		return screenShakeAmplitude * decay * Math::Cos(age * screenShakeFrequency);
	}

	void AvoidanceManager::requestScreenShake(
		double amplitude,
		int32 durationStep,
		double frequency) {

		screenShakeActive = true;
		screenShakeStopwatch.restart();
		screenShakeAmplitude = amplitude;
		screenShakeDurationStep = Max(durationStep, 1);
		screenShakeFrequency = frequency;
	}

	double AvoidanceManager::getChapterTransitionFadeAlpha() const {
		if (!chapterTransitionFadeActive) {
			return 0.0;
		}

		const double age = chapterTransitionFadeStopwatch.sF() * 50.0;
		if (chapterTransitionFadeDurationStep < age) {
			return 0.0;
		}

		const double t = age / static_cast<double>(Max(chapterTransitionFadeDurationStep, 1));
		return 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
	}

	void AvoidanceManager::requestChapterTransitionFade(int32 durationStep) {
		chapterTransitionFadeActive = true;
		chapterTransitionFadeStopwatch.restart();
		chapterTransitionFadeDurationStep = Max(durationStep, 1);
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

	int32 AvoidanceManager::getActiveChapter() const {
		return activeChapter;
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
		markDrawListDirty();
	}

	void AvoidanceManager::createCherry(const Vec2& pos, const Cherry::Settings& settings) {
		gameObjects.cherries << acquireCherry(pos, settings);
		markDrawListDirty();
	}

	std::shared_ptr<Cherry> AvoidanceManager::acquireCherry(
		const Vec2& pos,
		const Cherry::Settings& settings) {

		std::shared_ptr<Cherry> cherry;
		if (inactiveCherries.isEmpty()) {
			cherry = std::make_shared<Cherry>();
		}
		else {
			cherry = inactiveCherries.back();
			inactiveCherries.pop_back();
		}

		cherry->reset(pos, settings);
		return cherry;
	}

	void AvoidanceManager::recycleCherry(const std::shared_ptr<Cherry>& cherry) {
		cherry->deactivate();
		constexpr size_t cherryPoolMaxInactiveCount = 2048;
		if (inactiveCherries.size() < cherryPoolMaxInactiveCount) {
			inactiveCherries << cherry;
		}
	}

	void AvoidanceManager::recycleAllCherries() {
		for (const auto& cherry : gameObjects.cherries) {
			recycleCherry(cherry);
		}
		gameObjects.cherries.clear();
		markDrawListDirty();
	}

	void AvoidanceManager::markDrawListDirty() {
		drawListDirty = true;
	}

	void AvoidanceManager::rebuildDrawListIfNeeded() const {
		if (!drawListDirty) {
			return;
		}

		sortedDrawList.clear();
		sortedDrawList.reserve(2
			+ gameObjects.blocks.size()
			+ gameObjects.cherries.size()
			+ gameObjects.bloods.size()
			+ gameObjects.bullets.size());

		sortedDrawList << gameObjects.miku.get();
		sortedDrawList << gameObjects.player.get();
		for (const auto& block : gameObjects.blocks) sortedDrawList << block.get();
		for (const auto& cherry : gameObjects.cherries) sortedDrawList << cherry.get();
		for (const auto& blood : gameObjects.bloods) sortedDrawList << blood.get();
		for (const auto& bullet : gameObjects.bullets) sortedDrawList << bullet.get();

		std::stable_sort(sortedDrawList.begin(), sortedDrawList.end(), [](const GameObject* a, const GameObject* b) {
			return a->getDepth() < b->getDepth();
		});
		drawListDirty = false;
	}

	//外周のブロック配置
	void AvoidanceManager::createPeripheryBlocks() {
		Array<BlockPlacement> blockSettings;
		addPeripheryBlockSettings(blockSettings);

		for (const auto& blockSetting : blockSettings) {
			auto block = std::make_shared<Block>(blockSetting.textureName, blockSetting.gridPos);
			block->setHasCollide(blockSetting.hasCollide);
			gameObjects.blocks << block;
		}
		markDrawListDirty();
	}

	//5マス分の床ブロックを作成
	void AvoidanceManager::createFloorBlocks(Vec2 basePos) {
		Array<BlockPlacement> blockSettings;
		addFloorBlockSettings(blockSettings, basePos);

		for (const auto& blockSetting : blockSettings) {
			auto block = std::make_shared<Block>(blockSetting.textureName, blockSetting.gridPos);
			block->setHasCollide(blockSetting.hasCollide);
			gameObjects.blocks << block;
		}
		markDrawListDirty();
	}
}
