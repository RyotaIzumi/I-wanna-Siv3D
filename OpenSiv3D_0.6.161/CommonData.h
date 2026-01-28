#pragma once
#include "MainGame.h"
//#include "SaveData.h"

namespace Iwanna {
	enum class SceneType;

	struct CommonData {
		MainGame game;
		//SaveData globalSaveData;
		SceneManager<SceneType, CommonData>* sceneManager;
	};
}
