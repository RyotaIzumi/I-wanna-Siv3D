#pragma once
#include <Siv3D.hpp>
#include "../GameObject/StockNearGameObjects.h"
#include "../GameObject/Player.h"
#include "../GameObject/Bullet.h"
#include "../GameObject/Cherry.h"
#include "../GameObject/Block.h"
#include "../GameObject/Spike.h"
#include "../GameObject/Trigger.h"
#include "../Global.h"

namespace Iwanna {

	struct StageGameObjects {
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Bullet>> bullets;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		Array<std::shared_ptr<Spike>> spikes;
		Array<std::shared_ptr<Trigger>> triggers;

	};

	class StageManager {
	private:
		StockNearGameObjects stockNearGameObjects;
		StockNearGameObjects stockBulletsNearGameObjects;
		StageGameObjects gameObjects;

		//弾丸関連
		double bulletSpeed = 8;
		int32 bulletMaxNum = 5;

		int32 oneTileSize = 32;

		int32 step = 0;
	public:
		StageManager();

		void setUpObjects(int32 chapter);
		void loadGameObjects(String);
		Vec2 parsePos(const JSON& json);

		void update();
		void debug();
		void draw() const;
		void setStep(int32 newStep);

		std::shared_ptr<Player> getPlayer();
		Array<std::shared_ptr<Cherry>> getCherries();
		Array<std::shared_ptr<Block>> getBlocks();

		void createCherry(std::shared_ptr<Cherry> cherry);

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);
	};
}
