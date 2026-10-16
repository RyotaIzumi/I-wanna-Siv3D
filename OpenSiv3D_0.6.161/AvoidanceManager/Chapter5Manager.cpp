#include "AvoidanceManager.h"

namespace Iwanna {
	void AvoidanceManager::chapter5() {
		const int32 beginStep = Global::startStep_Chapter5;
		const Timeline timeline(Max(previousStep - beginStep, -1), step - beginStep);
		chapter5Script.update(*this, timeline);
	}
}
