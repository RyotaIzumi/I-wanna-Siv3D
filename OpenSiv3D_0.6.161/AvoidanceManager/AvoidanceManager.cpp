#include "AvoidanceManager.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna {
	AvoidanceManager::AvoidanceManager() {
		stockNearGameObjects.cellSize = 64;
		stockBulletsNearGameObjects.cellSize = 32;
	}

	void AvoidanceManager::setUpObjects(int32 chapter) {
		gameObjects.player = std::make_shared<Player>();
		currentChapter = chapter;
		Global::isInfiniteJumpMode = false;
		isTraversalStage = false;
		isTraversalCleared = false;
		disposablePlatformCount = 0;
		traversalGraphVertices.clear();
		traversalGraphEdges.clear();
		isSatStage = false;
		isSatCleared = false;
		satAssignment.clear();
		satClauses.clear();
		satClauseWaveSpawned.clear();

		gameObjects.cherries.clear();
		gameObjects.bullets.clear();
		gameObjects.bloods.clear();

		switch (chapter) {
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

			gameObjects.miku = std::make_shared<Miku>(Vec2{ 704,352 });
			break;
		case 20:
			createDisposableTraversalStage();
			gameObjects.miku = std::make_shared<Miku>(Vec2{ 1200,1200 });
			gameObjects.miku->canPlayerKill = false;
			break;
		case 21:
			createSatStage();
			gameObjects.miku = std::make_shared<Miku>(Vec2{ 1200,1200 });
			gameObjects.miku->canPlayerKill = false;
			break;
		}

		// 一部変数の初期化
		isGenerateBloods = false;
	}

	void AvoidanceManager::update() {
		if (isTraversalCleared || isSatCleared) return;

		//チャプターごとの更新処理
		if (currentChapter == 21) chapter21();
		else if (step < Global::startStep_Chapter2) chapter1();
		else if(step < Global::startStep_Chapter3) chapter2();

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
			b->update();
			stockNearGameObjects.add(b.get());
			if (b->getHasCollide()) stockBulletsNearGameObjects.add(b.get());
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
		if (isTraversalStage) updateDisposableTraversalStage();

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
		if (isTraversalStage) {
			const int32 visited = static_cast<int32>(gameObjects.blocks.count_if([](const auto& block) {
				return block->isDisposable() && block->isVisited();
			}));
			Print << U" Platforms : " << visited << U" / " << disposablePlatformCount;
		}
		if (isSatStage) Print << U" SAT Assignment : " << satAssignment;
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

		if (isTraversalStage) {
			const int32 visited = static_cast<int32>(gameObjects.blocks.count_if([](const auto& block) {
				return block->isDisposable() && block->isVisited();
			}));
			FontAsset(U"Button")(U"ONE-SHOT PLATFORM TOUR  ", visited, U" / ", disposablePlatformCount)
				.draw(20, 16, ColorF{ 0.08, 0.10, 0.16 });
			FontAsset(U"Button")(U"Visit every yellow platform, then reach the green flag.")
				.draw(20, 45, ColorF{ 0.15, 0.17, 0.22 });
			if (isTraversalCleared) {
				Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(ColorF{ 0.02, 0.08, 0.04, 0.72 });
				FontAsset(U"Big")(U"CLEAR!").drawAt(400, 270, ColorF{ 0.45, 1.0, 0.55 });
				FontAsset(U"Button")(U"All vertices visited. Press R to return.").drawAt(400, 340, Palette::White);
			}
		}

		if (isSatStage) {
			if (step < 450) {
				const int32 variableIndex = Min(step / 150, 2);
				RectF{ 32, 80, 368, 496 }.draw(ColorF{ 0.20, 0.75, 1.0, 0.10 });
				RectF{ 400, 80, 368, 496 }.draw(ColorF{ 1.0, 0.30, 0.45, 0.10 });
				Line{ 400, 80, 400, 576 }.draw(3, ColorF{ 0.15, 0.17, 0.22, 0.65 });
				FontAsset(U"Big")(U"TRUE").drawAt(215, 160, ColorF{ 0.15, 0.60, 0.95, 0.55 });
				FontAsset(U"Big")(U"FALSE").drawAt(585, 160, ColorF{ 0.95, 0.20, 0.35, 0.55 });
				FontAsset(U"Button")(U"Choose x", variableIndex + 1, U" before the timer ends")
					.drawAt(400, 92, ColorF{ 0.08, 0.10, 0.16 });
				const double remaining = 1.0 - static_cast<double>(step % 150) / 150.0;
				RectF{ 200, 120, 400 * remaining, 8 }.draw(ColorF{ 0.25, 0.90, 0.35 });
			}

			String assignmentText = U"Assignment: ";
			for (int32 i = 0; i < static_cast<int32>(satAssignment.size()); ++i) {
				assignmentText += U"x" + Format(i + 1) + U"=";
				assignmentText += (satAssignment[i] < 0 ? U"?" : (satAssignment[i] == 1 ? U"T" : U"F"));
				assignmentText += U"  ";
			}
			FontAsset(U"Button")(assignmentText).draw(20, 16, ColorF{ 0.08, 0.10, 0.16 });

			if (step >= 450) {
				const int32 clauseIndex = Clamp((step - 520) / 190, 0, 3);
				FontAsset(U"Button")(U"Clause verification ", clauseIndex + 1, U" / 4")
					.draw(20, 45, ColorF{ 0.15, 0.17, 0.22 });
				const Array<int32> laneCenters = { 160, 400, 640 };
				for (int32 lane = 0; lane < 3; ++lane) {
					const bool open = isSatLiteralTrue(satClauses[clauseIndex][lane]);
					RectF{ laneCenters[lane] - 48, 76, 96, 28 }
						.draw(open ? ColorF{ 0.20, 0.90, 0.35, 0.85 } : ColorF{ 0.95, 0.18, 0.25, 0.85 });
					const String literalName = (satClauses[clauseIndex][lane] > 0 ? U"x" : U"not x")
						+ Format(Abs(satClauses[clauseIndex][lane]));
					FontAsset(U"Button")(literalName).drawAt(laneCenters[lane], 78, Palette::White);
				}
			}

			if (isSatCleared) {
				Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(ColorF{ 0.02, 0.08, 0.04, 0.72 });
				FontAsset(U"Big")(U"SAT CLEAR!").drawAt(400, 270, ColorF{ 0.45, 1.0, 0.55 });
				FontAsset(U"Button")(U"Every clause had a true literal. Press R to return.")
					.drawAt(400, 340, Palette::White);
			}
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

	void AvoidanceManager::createDisposableTraversalStage() {
		isTraversalStage = true;
		gameObjects.blocks.clear();
		gameObjects.cherries.clear();
		traversalGraphVertices.clear();
		traversalGraphEdges.clear();
		traversalStaticCherryPositions.clear();
		traversalGuardCenterIndex = -1;

		// Keep the start and goal outside the staggered candidate rows shown in the reference.
		const Array<Point> candidates = {
			{ 2, 17 }, { 10, 17 }, { 18, 17 },
			{ 6, 14 }, { 14, 14 }, { 22, 14 },
			{ 2, 11 }, { 10, 11 }, { 18, 11 },
			{ 6, 8 }, { 14, 8 }, { 22, 8 },
			{ 10, 5 }, { 18, 5 },
			{ 22, 2 }
		};
		constexpr int32 startCandidate = 0;
		const int32 goalCandidate = static_cast<int32>(candidates.size()) - 1;

		const auto isHamiltonEdge = [&candidates](int32 from, int32 to) {
			const Point delta = candidates[to] - candidates[from];
			if (delta.x == 0 && delta.y != 0) return false;
			if (Abs(delta.x) > 5) return false;
			if (delta.y < 0) return Abs(delta.y) <= 3;
			return delta.y <= 6;
		};

		Array<Array<int32>> neighbors(candidates.size());
		for (int32 from = 0; from < static_cast<int32>(candidates.size()); ++from) {
			for (int32 to = 0; to < static_cast<int32>(candidates.size()); ++to) {
				if (from != to && isHamiltonEdge(from, to)) neighbors[from] << to;
			}
		}

		const auto isPotentialCandidateRoute = [&candidates](const Array<int32>& path, int32 from, int32 to) {
			const Point delta = candidates[path[to]] - candidates[path[from]];
			if (delta.y < 0) {
				return Abs(delta.x) <= 5 && Abs(delta.y) <= 3;
			}
			return Abs(delta.x) <= Max(5, 2 + delta.y * 2) && delta.y <= 15;
		};
		const auto canBecomeBranch = [&](const Array<int32>& path, int32 from, int32 to) {
			const Point delta = candidates[path[to]] - candidates[path[from]];
			return isPotentialCandidateRoute(path, from, to)
				&& delta.y <= 6
				&& !(delta.x == 0 && delta.y != 0);
		};
		const auto countBranchableSkips = [&](const Array<int32>& path) {
			int32 count = 0;
			for (int32 from = 0; from < static_cast<int32>(path.size()); ++from) {
				for (int32 to = from + 2; to < static_cast<int32>(path.size()); ++to) {
					if (canBecomeBranch(path, from, to)) ++count;
				}
			}
			return count;
		};

		Array<int32> selectedPath;
		const int32 targetPlatformCount = 10;
		const int32 targetPathSize = targetPlatformCount + 2;
		std::function<bool(int32)> findPath = [&](int32 current) {
			if (current == goalCandidate) return static_cast<int32>(selectedPath.size()) == targetPathSize;
			if (static_cast<int32>(selectedPath.size()) >= targetPathSize) return false;

			Array<int32> choices = neighbors[current];
			for (int32 i = static_cast<int32>(choices.size()) - 1; i > 0; --i) {
				std::swap(choices[i], choices[Random<int32>(0, i)]);
			}
			for (const int32 next : choices) {
				if (selectedPath.contains(next)) continue;
				if (next == goalCandidate
					&& static_cast<int32>(selectedPath.size()) != targetPathSize - 1) continue;
				selectedPath << next;
				if (findPath(next)) return true;
				selectedPath.pop_back();
			}
			return false;
		};

		bool foundPath = false;
		for (int32 attempt = 0; attempt < 120 && !foundPath; ++attempt) {
			selectedPath.clear();
			selectedPath << startCandidate;
			if (findPath(startCandidate) && countBranchableSkips(selectedPath) >= 2) {
				foundPath = true;
			}
		}
		if (!foundPath) {
			// This fixed fallback is also a Hamilton path on the same candidate lattice.
			selectedPath = { 0, 3, 1, 4, 7, 9, 12, 10, 8, 11, 13, 14 };
		}

		const Point startVertex = candidates[selectedPath.front()];
		const Point goalVertex = candidates[selectedPath.back()];
		gameObjects.blocks << std::make_shared<Block>(U"sprFloor", startVertex);
		gameObjects.player->pos = Vec2{ startVertex.x * 32 + 16, startVertex.y * 32 - 20 };

		for (int32 i = 1; i < static_cast<int32>(selectedPath.size()) - 1; ++i) {
			auto platform = std::make_shared<Block>(U"sprFloor", candidates[selectedPath[i]]);
			platform->makeDisposable();
			gameObjects.blocks << platform;
		}
		disposablePlatformCount = static_cast<int32>(selectedPath.size()) - 2;

		auto goal = std::make_shared<Block>(U"sprFloor", goalVertex);
		goal->makeGoal();
		gameObjects.blocks << goal;

		const auto centerOf = [](const Point& vertex) {
			return Vec2{ vertex * 32 } + Vec2{ 16, 16 };
		};
		for (const int32 candidate : selectedPath) {
			traversalGraphVertices << centerOf(candidates[candidate]);
		}
		for (int32 i = 0; i + 1 < static_cast<int32>(selectedPath.size()); ++i) {
			traversalGraphEdges << std::pair<int32, int32>{ i, i + 1 };
		}

		const auto isPotentialRoute = [&](int32 from, int32 to) {
			return isPotentialCandidateRoute(selectedPath, from, to);
		};

		Array<std::pair<int32, int32>> branchCandidates;
		for (int32 from = 0; from < static_cast<int32>(selectedPath.size()); ++from) {
			for (int32 to = from + 2; to < static_cast<int32>(selectedPath.size()); ++to) {
				if (canBecomeBranch(selectedPath, from, to)) {
					branchCandidates << std::pair<int32, int32>{ from, to };
				}
			}
		}
		for (int32 i = static_cast<int32>(branchCandidates.size()) - 1; i > 0; --i) {
			std::swap(branchCandidates[i], branchCandidates[Random<int32>(0, i)]);
		}
		const int32 branchCount = Min(Random<int32>(1, 2),
			Max(0, static_cast<int32>(branchCandidates.size()) - 1));
		for (int32 i = 0; i < branchCount; ++i) {
			traversalGraphEdges << branchCandidates[i];
		}

		const auto graphEdgeExists = [&](int32 from, int32 to) {
			return traversalGraphEdges.contains(std::pair<int32, int32>{ from, to })
				|| traversalGraphEdges.contains(std::pair<int32, int32>{ to, from });
		};
		const auto appendStaticCherryPosition = [&](const Vec2& position) {
			if (traversalStaticCherryPositions.any([&](const Vec2& existing) {
				return existing.distanceFrom(position) < 14.0;
			})) {
				return;
			}
			traversalStaticCherryPositions << position;
		};
		const auto appendStaticSegment = [&](const Vec2& start, const Vec2& end, double interval) {
			const double length = start.distanceFrom(end);
			const int32 divisions = Max(1, static_cast<int32>(Ceil(length / interval)));
			for (int32 i = 0; i <= divisions; ++i) {
				appendStaticCherryPosition(start.lerp(end, static_cast<double>(i) / divisions));
			}
		};

		for (int32 from = 0; from < static_cast<int32>(traversalGraphVertices.size()); ++from) {
			for (int32 to = from + 1; to < static_cast<int32>(traversalGraphVertices.size()); ++to) {
				if (graphEdgeExists(from, to)) continue;

				const Vec2 route = traversalGraphVertices[to] - traversalGraphVertices[from];
				if (route.lengthSq() == 0.0) continue;
				if (Abs(route.x) > 164.0 || Abs(route.y) > 124.0) continue;
				if (!isPotentialRoute(from, to) && !isPotentialRoute(to, from)) continue;

				const Vec2 middle = (traversalGraphVertices[from] + traversalGraphVertices[to]) * 0.5;
				const Vec2 direction = route.normalized();
				const Vec2 normal = Vec2{ -direction.y, direction.x };
				appendStaticSegment(middle - normal * 60.0,
					middle + normal * 60.0,
					20.0);
			}
		}

		for (const Vec2& position : traversalStaticCherryPositions) {
			auto cherry = std::make_shared<Cherry>();
			cherry->pos = position;
			createCherry(cherry);
		}
	}

	void AvoidanceManager::updateDisposableTraversalStage() {
		if (!isTraversalStage || gameObjects.player->getIsDead()) return;

		if (gameObjects.player->pos.y > Global::stageHeight + 48
			|| gameObjects.player->pos.x < -32
			|| gameObjects.player->pos.x > Global::stageWidth + 32) {
			gameObjects.player->playerDead();
			return;
		}

		const auto appendCherryPosition = [](Array<Vec2>& positions, const Vec2& position) {
			if (positions.any([&](const Vec2& existing) {
				return existing.distanceFrom(position) < 14.0;
			})) {
				return;
			}
			positions << position;
		};

		const auto appendSegment = [&](Array<Vec2>& positions, const Vec2& start, const Vec2& end, double interval) {
			const double length = start.distanceFrom(end);
			const int32 divisions = Max(1, static_cast<int32>(Ceil(length / interval)));
			for (int32 i = 0; i <= divisions; ++i) {
				appendCherryPosition(positions, start.lerp(end, static_cast<double>(i) / divisions));
			}
		};

		const auto rebuildTraversalGuard = [&](int32 centerIndex) {
			gameObjects.cherries.clear();

			Array<Vec2> guardPositions;
			for (const Vec2& position : traversalStaticCherryPositions) {
				appendCherryPosition(guardPositions, position);
			}

			const Vec2 center = traversalGraphVertices[centerIndex];
			constexpr double halfSize = 172.0;
			constexpr double interval = 18.0;

			appendSegment(guardPositions,
				center + Vec2{ -halfSize, -halfSize },
				center + Vec2{ halfSize, -halfSize }, interval);
			appendSegment(guardPositions,
				center + Vec2{ halfSize, -halfSize },
				center + Vec2{ halfSize, halfSize }, interval);
			appendSegment(guardPositions,
				center + Vec2{ halfSize, halfSize },
				center + Vec2{ -halfSize, halfSize }, interval);
			appendSegment(guardPositions,
				center + Vec2{ -halfSize, halfSize },
				center + Vec2{ -halfSize, -halfSize }, interval);

			for (const Vec2& position : guardPositions) {
				auto cherry = std::make_shared<Cherry>();
				cherry->pos = position;
				createCherry(cherry);
			}
		};

		if (gameObjects.player->getOnGround()) {
			const Vec2 playerFeet = gameObjects.player->hitBox->bottom();
			for (int32 i = 0; i < static_cast<int32>(gameObjects.blocks.size())
				&& i < static_cast<int32>(traversalGraphVertices.size()); ++i) {
				const auto& block = gameObjects.blocks[i];
				if (!block->getHasCollide()) continue;
				const auto* rect = block->hitBox->getRect();
				if (rect == nullptr) continue;
				const bool onThisBlock = (rect->x - 8.0 <= playerFeet.x
					&& playerFeet.x <= rect->x + rect->w + 8.0
					&& Abs(playerFeet.y - rect->y) <= 4.0);
				if (!onThisBlock) continue;
				if (traversalGuardCenterIndex != i) {
					traversalGuardCenterIndex = i;
					rebuildTraversalGuard(i);
				}
				break;
			}
		}

		const int32 visited = static_cast<int32>(gameObjects.blocks.count_if([](const auto& block) {
			return block->isDisposable() && block->isVisited();
		}));
		if (visited != disposablePlatformCount || !gameObjects.player->getOnGround()) return;

		const double playerFeet = gameObjects.player->hitBox->bottom().y;
		for (const auto& block : gameObjects.blocks) {
			if (!block->isGoal()) continue;
			const auto* rect = block->hitBox->getRect();
			if (rect && rect->x <= gameObjects.player->pos.x && gameObjects.player->pos.x <= rect->rightX()
				&& Abs(playerFeet - rect->y) <= 3.0) {
				isTraversalCleared = true;
				return;
			}
		}
	}

	void AvoidanceManager::createSatStage() {
		isSatStage = true;
		gameObjects.blocks.clear();
		createPeripheryBlocks();
		gameObjects.player->pos = Vec2{ 400, 550 };
		gameObjects.player->hitBox->setPos(gameObjects.player->pos);

		satAssignment = { -1, -1, -1 };
		// Positive values are xi; negative values are not xi.
		satClauses = {
			{ 1, -2, 3 },
			{ -1, 2, 3 },
			{ 1, 2, -3 },
			{ -1, -2, -3 }
		};
		satClauseWaveSpawned = { false, false, false, false };
	}

	bool AvoidanceManager::isSatLiteralTrue(int32 literal) const {
		const int32 index = Abs(literal) - 1;
		if (index < 0 || index >= static_cast<int32>(satAssignment.size()) || satAssignment[index] < 0) {
			return false;
		}
		return (literal > 0) ? (satAssignment[index] == 1) : (satAssignment[index] == 0);
	}

	void AvoidanceManager::createSatClauseWave(int32 clauseIndex) {
		const Array<int32> laneCenters = { 160, 400, 640 };
		for (int32 row = 0; row < 5; ++row) {
			for (int32 x = 40; x <= 760; x += 20) {
				bool isSafeGap = false;
				for (int32 lane = 0; lane < 3; ++lane) {
					if (isSatLiteralTrue(satClauses[clauseIndex][lane])
						&& Abs(x - laneCenters[lane]) <= 52) {
						isSafeGap = true;
					}
				}
				if (isSafeGap) continue;

				auto cherry = std::make_shared<Cherry>();
				cherry->pos = Vec2{ x, 36.0 + 20.0 * row };
				cherry->speed = 4.0;
				cherry->dir = 270.0;
				createCherry(cherry);
			}
		}
	}
}
