#pragma once
#include <Siv3D.hpp>
#include "GameObject/Player.h"
#include "GameObject/Cherry.h"
#include "GameObject/Block.h"
#include "GameObject/Miku.h"
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager.h"

namespace Iwanna {
	class MainGame {
	private:
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		std::shared_ptr<Miku> miku;

		StockNearGameObjects stockNearGameObjects;

		const int32 FPS = 50;
		Audio audio;

		AvoidanceManager avoidanceManager;

	public:
		MainGame();

		void startGame();
		void updateGame();
		void debugGame();
		void drawGame();

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);

		void playBgm(int32 chapter);
		void stopBgm();

		void pauseBgm();
	};
}
