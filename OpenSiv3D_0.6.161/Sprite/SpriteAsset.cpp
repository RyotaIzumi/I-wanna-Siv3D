#include "SpriteAsset.h"

namespace Iwanna {
	Array<std::pair<String, String>> registry;

	void registerTexture(const String& name, const String& path) {
		registry << std::pair<String, String>(name, path);
	}

	void registerTextures() {

		//Player
		static const FilePath PlayerPath = U"Texture/Player/";
		registerTexture(U"sprPlayerIdle", PlayerPath + U"sprPlayerIdle.png");
		registerTexture(U"sprPlayerFall", PlayerPath + U"sprPlayerFall.png");
		registerTexture(U"sprPlayerJump", PlayerPath + U"sprPlayerJump.png");
		registerTexture(U"sprPlayerRunning", PlayerPath + U"sprPlayerRunning.png");
	}


	void registerTexturesSync() {
		for (auto& p : registry) {
			TextureAsset::Register(p.first, p.second);
		}
	}

	void loadTexturesSync() {
		for (auto& p : registry) {
			TextureAsset::Load(p.first);
		}
	}

}
