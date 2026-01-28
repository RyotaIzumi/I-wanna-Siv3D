#include "MainGame.h"

namespace Iwanna {
	MainGame::MainGame() {
		player = std::make_shared<Player>();

		SizeF blockSize{ 32,32 };
		//仮ブロック配置
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

	void MainGame::debugGame() {

	}

	void MainGame::drawGame() {
		//背景描画
		Rect(0, 0, 800, 600).draw(ColorF(0.8, 1.0));
		//kid君描画
		player->draw();
		//ブロック描画
		for (auto b : blocks) {
			b->draw(Palette::Gray);
		}
	}
}
