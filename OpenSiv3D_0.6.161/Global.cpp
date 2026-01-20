#include "Global.h"

namespace Global {

	bool jumpDownFrame = false;
	bool jumpUpFrame = false;
	bool shootDownFrame = false;

	void captureInputFrame()
	{
		jumpDownFrame = inputJump.down();
		jumpUpFrame = inputJump.up();
		shootDownFrame = inputShoot.down();
	}
}
