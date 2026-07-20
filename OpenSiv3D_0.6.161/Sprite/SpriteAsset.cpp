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

		//実績アイコン登録
		static const FilePath IconPath = U"Texture/Icon/";
		registerTexture(U"achive_locked", IconPath + U"achive_locked.png");
		registerTexture(U"achive_start", IconPath + U"achive_start.png");
		registerTexture(U"achive_chapter1", IconPath + U"achive_chapter1.png");
		registerTexture(U"achive_chapter2", IconPath + U"achive_chapter2.png");
		registerTexture(U"achive_chapter3", IconPath + U"achive_chapter3.png");
		registerTexture(U"achive_chapter4", IconPath + U"achive_chapter4.png");
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
