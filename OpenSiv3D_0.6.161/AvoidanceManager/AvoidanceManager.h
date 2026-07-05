#pragma once
#include <Siv3D.hpp>
#include "../GameObject/StockNearGameObjects.h"
#include "../GameObject/Player.h"
#include "../GameObject/Bullet.h"
#include "../GameObject/Cherry.h"
#include "../GameObject/Block.h"
#include "../GameObject/Blood.h"
#include "../GameObject/Miku.h"
#include "../Global.h"
#include "EasingMove.h"
#include "Timeline.h"

namespace Iwanna {

	struct GameObjects {
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Bullet>> bullets;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		Array<std::shared_ptr<Blood>> bloods;
		std::shared_ptr<Miku> miku;
	};

	struct BlockPlacement {
		String textureName = U"sprBlock";
		Vec2 gridPos = Vec2{ 0,0 };
		bool hasCollide = true;
		double depth = DrawDepth::Block;
	};

	struct ChapterSettings {
		Vec2 playerPos = Vec2{ 200,500 };
		Array<BlockPlacement> blocks;
		ColorF backgroundColor = ColorF(0.8, 1.0);
		Vec2 mikuPos = Vec2{ 704,352 };
		bool isInfiniteJumpMode = false;
		double playerDepth = DrawDepth::Player;
		double mikuDepth = DrawDepth::Miku;
	};

	class AvoidanceManager {
	private:
		StockNearGameObjects stockNearGameObjects;
		StockNearGameObjects stockBulletsNearGameObjects;
		GameObjects gameObjects;
		Array<std::shared_ptr<Cherry>> inactiveCherries;
		mutable Array<GameObject*> sortedDrawList;
		mutable bool drawListDirty = true;
		ColorF backgroundColor = ColorF(0.8, 1.0);

		//弾丸関連
		double bulletSpeed = 8;
		int32 bulletMaxNum = 5;

		//血しぶき数
		int32 bloodNum = 80;
		//血を生成したかどうか
		bool isGenerateBloods = false;

		int32 previousStep = -1;
		int32 step = 0;
		int32 activeChapter = 0;

		int32 getChapterFromStep(int32 targetStep) const;
		ChapterSettings createChapterSettings(int32 chapter) const;
		void applyChapterSettings(const ChapterSettings& settings);
		void addPeripheryBlockSettings(Array<BlockPlacement>& blocks) const;
		void addFloorBlockSettings(Array<BlockPlacement>& blocks, Vec2 basePos) const;
		std::shared_ptr<Cherry> acquireCherry(const Vec2& pos, const Cherry::Settings& settings);
		void recycleCherry(const std::shared_ptr<Cherry>& cherry);
		void recycleAllCherries();
		void markDrawListDirty();
		void rebuildDrawListIfNeeded() const;
	public:
		AvoidanceManager();

		void setUpObjects(int32 chapter);
		void update();
		void debug();
		void draw() const;
		void setStep(int32 newStep);

		std::shared_ptr<Player> getPlayer();
		Array<std::shared_ptr<Cherry>> getCherries();
		Array<std::shared_ptr<Block>> getBlocks();
		std::shared_ptr<Miku> getMiku();

		void createCherry(std::shared_ptr<Cherry> cherry);
		void createCherry(const Vec2& pos, const Cherry::Settings& settings);
		std::function<std::shared_ptr<Cherry>()> makeCherryFactory(const Cherry::Settings& settings);
		std::function<std::shared_ptr<Cherry>()> makeCherryFactory(
			const String& textureName,
			const ColorF& color,
			Cherry::Behavior behavior = nullptr);

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);

		void chapter1();
		void chapter2();
		void chapter3();
		void chapter4();
		void chapter5();
		void chapter6();

		//cherry生成パターン
		void createCherrySpread(Vec2 pos, int32 num, double spd, const std::function<std::shared_ptr<Cherry>()>& factory);
	};
}
