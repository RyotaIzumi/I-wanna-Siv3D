#pragma once
#include <Siv3D.hpp>
#include "Global.h"

namespace Iwanna {
	struct ReplayInputFrame {
		bool leftPressed = false;
		bool rightPressed = false;
		bool jumpDown = false;
		bool jumpUp = false;
		bool shootDown = false;

		static ReplayInputFrame FromCurrentInput() {
			ReplayInputFrame frame;
			frame.leftPressed = Global::inputLeft.pressed();
			frame.rightPressed = Global::inputRight.pressed();
			frame.jumpDown = Global::inputJump.down();
			frame.jumpUp = Global::inputJump.up();
			frame.shootDown = Global::inputShoot.down();
			return frame;
		}
	};

	struct ReplayData {
		int32 chapter = 1;
		int32 startStep = 0;
		int32 fps = Global::FPS;
		uint64 randomSeed = 0;
		Global::Difficulty difficulty = Global::Difficulty::Unselected;
		Array<ReplayInputFrame> frames;
		Array<int32> frameSteps;

		bool isValid() const {
			return !frames.isEmpty()
				&& frames.size() == frameSteps.size();
		}
	};
}
