#include "AvoidanceManager.h"

namespace Iwanna {
	AvoidanceManager::AvoidanceManager() {

	}

	void AvoidanceManager::update() {
		ClearPrint();
		Print << U"Avoidance Step: " << step;
	}

	void AvoidanceManager::setStep(int32 newStep) {
		step = newStep;
	}
}
