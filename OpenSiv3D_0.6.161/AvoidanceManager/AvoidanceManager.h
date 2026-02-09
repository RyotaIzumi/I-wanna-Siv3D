#pragma once
#include <Siv3D.hpp>
#include "../GameObject/Player.h"
#include "../GameObject/Cherry.h"
#include "../GameObject/Block.h"
#include "../GameObject/Miku.h"
#include "../Global.h"

namespace Iwanna {

	struct GameObjects {
		std::shared_ptr<Player> player;
		Array<std::shared_ptr<Cherry>> cherries;
		Array<std::shared_ptr<Block>> blocks;
		std::shared_ptr<Miku> miku;
	};

	class AvoidanceManager {
	private:

		GameObjects gameObjects;

		int32 step = 0;
	public:
		AvoidanceManager();

		void update();
		void setStep(int32 newStep);

		std::shared_ptr<Player> getPlayer();
		Array<std::shared_ptr<Cherry>> getCherries();
		Array<std::shared_ptr<Block>> getBlocks();
		std::shared_ptr<Miku> getMiku();

		void createPeripheryBlocks();
		void createFloorBlocks(Vec2 basePos);

		void chapter1();
		void chapter2();
	};
}
