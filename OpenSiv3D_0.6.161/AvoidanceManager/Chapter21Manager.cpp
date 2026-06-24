#include "AvoidanceManager.h"

namespace Iwanna {
	void AvoidanceManager::chapter21() {
		if (!isSatStage || gameObjects.player->getIsDead()) return;

		const Array<int32> assignmentDeadlines = { 150, 300, 450 };
		for (int32 variable = 0; variable < 3; ++variable) {
			if (satAssignment[variable] < 0 && step >= assignmentDeadlines[variable]) {
				// Left half is True; right half is False.
				satAssignment[variable] = (gameObjects.player->pos.x < 400.0 ? 1 : 0);
				if (variable < 2) {
					gameObjects.player->pos = Vec2{ 400, 550 };
					gameObjects.player->vspeed = 0;
					gameObjects.player->hitBox->setPos(gameObjects.player->pos);
				}
				break;
			}
		}

		const Array<int32> waveStartSteps = { 520, 710, 900, 1090 };
		for (int32 clause = 0; clause < 4; ++clause) {
			if (!satClauseWaveSpawned[clause] && step >= waveStartSteps[clause]) {
				createSatClauseWave(clause);
				satClauseWaveSpawned[clause] = true;
			}
		}

		if (step >= 1300) {
			isSatCleared = true;
		}
	}
}
