#pragma once
#include <Siv3D.hpp>
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager/AvoidanceManager.h"
#include "StageManager/StageManager.h"	

namespace Iwanna {
	class MainGame {
	private:
		StockNearGameObjects stockNearGameObjects;

		Audio audio;

		AvoidanceManager avoidanceManager;
		StageManager stageManager;

	public:
		MainGame();

		void startGame();
		void updateGame();
		void debugGame();
		void drawGame();

		void playBgm(int32 chapter);
		void stopBgm();

		void pauseBgm();
	};
}
