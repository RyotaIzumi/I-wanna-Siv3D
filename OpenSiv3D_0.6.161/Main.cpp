#include <Siv3D.hpp> // Siv3D v0.6.16
#include "Sprite/SpriteAsset.h"
//#include "AudioAsset.h"
#include "Scene/Scene.h"

using App = SceneManager<Iwanna::SceneType, Iwanna::CommonData>;

void Main()
{
	Window::SetTitle(U"I wanna Siv3D");
	Scene::SetResizeMode(ResizeMode::Keep);
	bool isFullScreen = true;

	//Escで終了しないように
	//System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	//フォントはここで宣言
	FontAsset::Register(U"Font", 60, Typeface::Regular);
	FontAsset::Register(U"Button", FontMethod::MSDF, 20, Typeface::Medium);

	
	Iwanna::registerTextures();
	//Minge::Sound::registerAudios();
	//Minge::Sound::registerSEs();
	

	App app;
	//app.get()->game.commonData = app.get().get();
	//app.get()->sceneManager = &app;
	app.add<Iwanna::Loading>(Iwanna::SceneType::LOADING);
	//app.add<Iwanna::Title>(Iwanna::SceneType::TITLE);
	app.add<Iwanna::InGame>(Iwanna::SceneType::IN_GAME);
	app.init(Iwanna::SceneType::LOADING, 0s);

	while (System::Update()) {
		Cursor::RequestStyle(U"normal");

		//スクリーン設定
		if (KeyF4.down())isFullScreen = !isFullScreen;
		Window::SetFullscreen(isFullScreen);

		if (not app.update()) {
			break;
		}
	}
}
