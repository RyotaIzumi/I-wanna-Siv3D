#include <Siv3D.hpp> // Siv3D v0.6.16
#include "Sprite/SpriteAsset.h"
#include "Audio/AudioAsset.h"
#include "Scene/Scene.h"
#include "60FPSwithAutoFrameSkip.h"

using App = SceneManager<Iwanna::SceneType, Iwanna::CommonData>;

void Main()
{
	System60::SetDisplaySize(DisplayResolution::SVGA_800x600);

	Window::SetTitle(U"I wanna Siv3D");

	//フォントはここで宣言
	FontAsset::Register(U"Big", 60, Typeface::Regular);
	FontAsset::Register(U"Button", FontMethod::MSDF, 20, Typeface::Medium);

	
	Iwanna::registerTextures();
	Iwanna::registerTexturesSync();
	Iwanna::loadTexturesSync();

	Iwanna::Sound::registerBGMs();
	Iwanna::Sound::registerSEs();
	Iwanna::Sound::registerAudiosSync();
	Iwanna::Sound::loadAudiosSync();

	App app;
	app.add<Iwanna::StartMenu>(Iwanna::SceneType::START_MENU);
	app.add<Iwanna::InGame>(Iwanna::SceneType::IN_GAME);
	app.init(Iwanna::SceneType::START_MENU, 0s);

	while (System60::Update()) {
		if (not app.update()) {
			break;
		}
	}
}
