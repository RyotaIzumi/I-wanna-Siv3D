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

namespace Iwanna {

	struct GameObjects {
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Bullet>> bullets;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		Array<std::shared_ptr<Blood>> bloods;
		std::shared_ptr<Miku> miku;
	};

	class AvoidanceManager {
	private:
		StockNearGameObjects stockNearGameObjects;
		StockNearGameObjects stockBulletsNearGameObjects;
		GameObjects gameObjects;

		//弾丸関連
		double bulletSpeed = 8;
		int32 bulletMaxNum = 5;

		//血しぶき数
		int32 bloodNum = 80;
		//血を生成したかどうか
		bool isGenerateBloods = false;
		bool isTraversalStage = false;
		bool isTraversalCleared = false;
		int32 disposablePlatformCount = 0;

		int32 step = 0;
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

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);
		void createDisposableTraversalStage();
		void updateDisposableTraversalStage();

		void chapter1();
		void chapter2();
		//void chapter3();
		//void chapter4();
		//void chapter5();
		void chapter6();

		//cherry生成パターン
		void createCherrySpread(Vec2 pos, int32 num, double spd, const std::function<std::shared_ptr<Cherry>()>& factory);
	};
}
