#pragma once
#include <Siv3D.hpp>
#include "GameObject/Player.h"
#include "GameObject/Cherry.h"
#include "GameObject/Block.h"
#include "GameObject/Miku.h"
#include "GameObject/StockNearGameObjects.h"

namespace Iwanna {
	class MainGame {
	private:
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		std::shared_ptr<Miku> miku;

		StockNearGameObjects stockNearGameObjects;

	public:
		MainGame();

		void startGame();
		void updateGame();
		void debugGame();
		void drawGame();

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);
	};
}
