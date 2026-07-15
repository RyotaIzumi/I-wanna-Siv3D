#pragma once
#include <Siv3D.hpp>
#include "GameObject/StockNearGameObjects.h"
#include "AvoidanceManager/AvoidanceManager.h"

namespace Iwanna {
	class MainGame {
	private:
		StockNearGameObjects stockNearGameObjects;

		Audio audio;

		AvoidanceManager avoidanceManager;
		int32 lastSelectedChapter = 2;

	public:
		MainGame();

		void startGame(int32 chapter = 2);
		int32 getLastSelectedChapter() const;
		void updateGame();
		void debugGame();
		void drawGame();

		void playBgm(int32 chapter);
		void stopBgm();

		void pauseBgm();
	};
}
