#include "SpriteAsset.h"

namespace Iwanna {
	Array<std::pair<String, String>> registry;

	void registerTexture(const String& name, const String& path) {
		registry << std::pair<String, String>(name, path);
	}

	void registerTextures() {

		//kid君のテクスチャ登録
		static const FilePath PlayerPath = U"Texture/Player/";
		registerTexture(U"sprPlayerIdle", PlayerPath + U"sprPlayerIdle.png");
		registerTexture(U"sprPlayerFall", PlayerPath + U"sprPlayerFall.png");
		registerTexture(U"sprPlayerJump", PlayerPath + U"sprPlayerJump.png");
		registerTexture(U"sprPlayerRunning", PlayerPath + U"sprPlayerRunning.png");
		registerTexture(U"sprBullet", PlayerPath + U"bullet.png");

		//さくらんぼテクスチャ登録
		static const FilePath CherryPath = U"Texture/Cherry/";
		registerTexture(U"sprCherry", CherryPath + U"sprCherry.png");
		registerTexture(U"sprCherryAllWhite", CherryPath + U"sprCherryAllWhite.png");
		registerTexture(U"sprCherryFrameWhite", CherryPath + U"sprCherryFrameWhite.png");
		registerTexture(U"sprCherryWhite", CherryPath + U"sprCherryWhite.png");

		//ブロックテクスチャ登録
		static const FilePath BlockPath = U"Texture/Block/";
		registerTexture(U"sprBlock", BlockPath + U"sprBlock.png");
		registerTexture(U"sprFloor", BlockPath + U"sprFloor.png");
		registerTexture(U"sprWall", BlockPath + U"sprWall.png");

		//mikuテクスチャ登録
		static const FilePath MikuPath = U"Texture/Miku/";
		registerTexture(U"sprMiku", MikuPath + U"sprMiku.png");

		//針テクスチャ登録
		static const FilePath SpikePath = U"Texture/Spike/";
		registerTexture(U"sprSpikeUp", SpikePath + U"sprSpikeUp.png");
		registerTexture(U"sprSpikeLeft", SpikePath + U"sprSpikeLeft.png");
		registerTexture(U"sprSpikeDown", SpikePath + U"sprSpikeDown.png");
		registerTexture(U"sprSpikeRight", SpikePath + U"sprSpikeRight.png");
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
