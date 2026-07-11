#include "Scene.h"

namespace Iwanna {
	StartMenu::StartMenu(const InitData& data) : IScene(data) {

	}

	void StartMenu::update() {
		auto& data = getData().game;

		for (int32 chapter = 1; chapter <= 6; ++chapter) {
			const RectF button{
				160.0 + (chapter - 1) * 84.0,
				330.0,
				72.0,
				48.0,
			};

			if (button.leftClicked()) {
				selectedChapter = chapter;
			}
		}

		const RectF startButton{ 300.0, 420.0, 200.0, 56.0 };
		if (startButton.leftClicked()) {
			data.startGame(selectedChapter);
			changeScene(SceneType::IN_GAME, 0.0s);
			return;
		}

		if (Global::inputStart.down()) {
			data.startGame(selectedChapter);
			changeScene(SceneType::IN_GAME, 0.0s);
		}
	}

	void StartMenu::draw() const {
		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(ColorF{ 0.08, 0.08, 0.10 });
		FontAsset(U"Big")(U"I wanna Siv3D").drawAt(400, 150, Palette::White);
		FontAsset(U"Button")(U"Select Chapter").drawAt(400, 270, ColorF{ 0.85, 0.88, 0.95 });

		for (int32 chapter = 1; chapter <= 6; ++chapter) {
			const RectF button{
				160.0 + (chapter - 1) * 84.0,
				330.0,
				72.0,
				48.0,
			};
			const bool selected = (chapter == selectedChapter);
			const bool hovered = button.mouseOver();
			const ColorF fill = selected
				? ColorF{ 0.25, 0.65, 1.0 }
				: (hovered ? ColorF{ 0.25, 0.25, 0.30 } : ColorF{ 0.16, 0.16, 0.20 });
			const ColorF frame = selected ? ColorF{ 0.75, 0.90, 1.0 } : ColorF{ 0.45, 0.45, 0.52 };

			button.rounded(6.0).draw(fill);
			button.rounded(6.0).drawFrame(2.0, frame);
			FontAsset(U"Button")(U"Ch " + Format(chapter)).drawAt(button.center(), Palette::White);
		}

		const RectF startButton{ 300.0, 420.0, 200.0, 56.0 };
		const ColorF startFill = startButton.mouseOver()
			? ColorF{ 0.95, 0.95, 1.0 }
			: ColorF{ 0.78, 0.82, 0.92 };
		startButton.rounded(6.0).draw(startFill);
		startButton.rounded(6.0).drawFrame(2.0, ColorF{ 0.25, 0.30, 0.42 });
		FontAsset(U"Button")(U"Start Chapter " + Format(selectedChapter)).drawAt(startButton.center(), ColorF{ 0.06, 0.07, 0.10 });

		FontAsset(U"Button")(U"Shift also starts the selected chapter").drawAt(400, 510, ColorF{ 0.60, 0.64, 0.72 });
	}
}
