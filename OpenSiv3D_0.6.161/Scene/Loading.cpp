#include "Scene.h"
#include "../Sprite/SpriteAsset.h"
//#include "../AudioAsset.h"

namespace Iwanna {
	Loading::Loading(const InitData& init) : IScene(init) {
		loadingTask = Async([this]() { load(); });
	}

	void Loading::load() {
		registerTexturesSync();
		//Sound::registerAudiosSync(profiler);
		loadTexturesSync();
		//Sound::loadAudiosSync(profiler);
		Timer timer{ 1s };
		timer.restart();
		while (!timer.reachedZero()) {
		}
	}

	void Loading::update() {
		if (loadingTask.isReady()) {
			changeScene(SceneType::IN_GAME, 0.25s);
		}
	}

	void Loading::draw() const {
		int32 w = Scene::Width();
		int32 h = Scene::Height();
		Rect{ 0, 0, w, h }.draw(Palette::Black);
		FontAsset(U"Big")(U"ロード中").drawAt(w / 2, h / 2 - 200);
	}
}
