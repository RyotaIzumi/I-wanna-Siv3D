#pragma once
#include <Siv3D.hpp>
#include "GameObject/Player.h"
#include "GameObject/Block.h"
#include "GameObject/StockNearGameObjects.h"

namespace Iwanna {
	class MainGame {
	private:
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Block>> blocks;
		StockNearGameObjects stockNearGameObjects;

	public:
		MainGame();

		void updateGame();
		void debugGame();
		void drawGame();
	};
}
