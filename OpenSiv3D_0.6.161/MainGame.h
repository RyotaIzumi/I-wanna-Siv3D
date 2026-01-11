#pragma once
#include <Siv3D.hpp>
#include "Player.h"
#include "HitBox.h"

namespace Iwanna {
	class MainGame {
	private:
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<HitBox>> blocks;
	public:
		MainGame();

		void updateGame();
		void drawGame();
	};
}
