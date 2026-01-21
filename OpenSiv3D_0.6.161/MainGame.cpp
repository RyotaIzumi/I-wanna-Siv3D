#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
		player = std::make_shared<Player>();
		blocks << std::make_shared<RectHitBox>(Vec2(160, 544), SizeF{ 32, 32 });
		blocks << std::make_shared<RectHitBox>(Vec2(160, 512), SizeF{ 32, 32 });
		blocks << std::make_shared<RectHitBox>(Vec2(160, 480), SizeF{ 32, 32 });
		blocks << std::make_shared<RectHitBox>(Vec2(160, 448), SizeF{ 32, 32 });
		blocks << std::make_shared<RectHitBox>(Vec2(320	, 448), SizeF{ 32, 32 });
		for(int i = 0;i < 25;i++){
			blocks << std::make_shared<RectHitBox>(Vec2(i * 32, 576), SizeF{ 32, 32 });
		}
	}

	void MainGame::updateGame() {
		player->update();
		for (auto b : blocks) {
			player->checkCollisionBlocks(b);
		}
		player->updateLate();
	}

	void MainGame::drawGame() {
		Rect(0, 0, 800, 600).draw(ColorF(0.8,1.0));
		player->draw();
		for (auto b : blocks) {
			b->draw(Palette::Gray);
		}
	}
}
