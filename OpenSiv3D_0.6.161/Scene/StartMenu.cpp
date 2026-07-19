#include "Scene.h"

namespace Iwanna {
	namespace {
		constexpr Vec2 ChapterSelectOffset = { 170.0, 30.0 };
		constexpr int32 ChapterButtonsPerRow = 5;
		constexpr double ChapterButtonBaseX = 190.0;
		constexpr double ChapterButtonBaseY = 330.0;
		constexpr double ChapterButtonSpacingX = 84.0;
		constexpr double ChapterButtonSpacingY = 56.0;
		constexpr int32 CameraMinPage = 0;
		constexpr int32 CameraMaxPage = 1;
		constexpr double CameraMoveDurationSec = 0.45;

		double easeOutCubic(double t) {
			const double clampedT = Clamp(t, 0.0, 1.0);
			return 1.0 - Math::Pow(1.0 - clampedT, 3.0);
		}

		RectF arrowButtonRect(int32 direction) {
			const double width = 52.0;
			const double height = 96.0;
			const double x = (direction < 0) ? 0.0 : (Global::windowWidth - width);
			return RectF{ x, (Global::windowHeight - height) * 0.5, width, height };
		}

		RectF chapterButtonRect(int32 chapter) {
			const int32 index = chapter - 1;
			const int32 column = index % ChapterButtonsPerRow;
			const int32 row = index / ChapterButtonsPerRow;
			return RectF{
				ChapterButtonBaseX + ChapterSelectOffset.x + column * ChapterButtonSpacingX,
				ChapterButtonBaseY + ChapterSelectOffset.y + row * ChapterButtonSpacingY,
				72.0,
				48.0,
			};
		}

		String twoDigits(int32 value) {
			return (value < 10 ? U"0" : U"") + Format(value);
		}

		String formatPlayTime(double seconds) {
			const int32 totalSeconds = Max(0, static_cast<int32>(Math::Floor(seconds)));
			const int32 hours = totalSeconds / 3600;
			const int32 minutes = (totalSeconds / 60) % 60;
			const int32 secs = totalSeconds % 60;
			return Format(hours) + U":" + twoDigits(minutes) + U":" + twoDigits(secs);
		}

		String formatEnduranceTime(double seconds) {
			const double safeSeconds = Max(0.0, seconds);
			const int32 minutes = static_cast<int32>(safeSeconds / 60.0);
			const double secs = safeSeconds - minutes * 60.0;
			return Format(minutes) + U":" + (secs < 10.0 ? U"0" : U"") + ToFixed(secs, 2);
		}

		void drawArrowButton(int32 direction, bool enabled) {
			const RectF button = arrowButtonRect(direction);
			const bool hovered = enabled && button.mouseOver();
			const ColorF fill = enabled
				? (hovered ? ColorF{ 0.28, 0.30, 0.38, 0.92 } : ColorF{ 0.18, 0.20, 0.27, 0.78 })
				: ColorF{ 0.10, 0.11, 0.15, 0.42 };
			const ColorF arrowColor = enabled ? ColorF{ 0.86, 0.90, 1.0 } : ColorF{ 0.36, 0.38, 0.44 };
			const Vec2 center = button.center();
			const double sign = static_cast<double>(direction);

			button.rounded(8.0).draw(fill);
			button.rounded(8.0).drawFrame(1.5, ColorF{ 0.42, 0.46, 0.58, enabled ? 0.75 : 0.35 });
			Triangle{
				Vec2{ center.x + sign * 13.0, center.y },
				Vec2{ center.x - sign * 10.0, center.y - 18.0 },
				Vec2{ center.x - sign * 10.0, center.y + 18.0 },
			}.draw(arrowColor);
		}
	}

	StartMenu::StartMenu(const InitData& data) : IScene(data) {
		const auto& game = getData().game;
		selectedChapter = Clamp(game.getLastSelectedChapter(), 1, game.getSaveData().highestChapter);
	}

	void StartMenu::updateCameraMove() {
		if (!isCameraMoving) {
			return;
		}

		cameraMoveTimerSec += Scene::DeltaTime();
		const double t = easeOutCubic(cameraMoveTimerSec / CameraMoveDurationSec);
		cameraX = Math::Lerp(cameraStartX, cameraTargetX, t);

		if (CameraMoveDurationSec <= cameraMoveTimerSec) {
			cameraX = cameraTargetX;
			isCameraMoving = false;
		}
	}

	void StartMenu::requestCameraMove(int32 direction) {
		if (isCameraMoving) {
			return;
		}

		const int32 nextPage = Clamp(cameraPage + direction, CameraMinPage, CameraMaxPage);
		if (nextPage == cameraPage) {
			return;
		}

		cameraPage = nextPage;
		cameraStartX = cameraX;
		cameraTargetX = cameraPage * Global::windowWidth;
		cameraMoveTimerSec = 0.0;
		isCameraMoving = true;
	}

	void StartMenu::update() {
		auto& data = getData().game;
		const int32 highestChapter = data.getSaveData().highestChapter;
		selectedChapter = Clamp(selectedChapter, 1, highestChapter);
		updateCameraMove();

		if (arrowButtonRect(-1).leftClicked()) {
			requestCameraMove(-1);
		}
		if (arrowButtonRect(1).leftClicked()) {
			requestCameraMove(1);
		}

		for (int32 chapter = 1; chapter <= 6; ++chapter) {
			const RectF button = chapterButtonRect(chapter).movedBy(-cameraX, 0.0);
			const bool unlocked = (chapter <= highestChapter);

			if (unlocked && button.leftClicked()) {
				selectedChapter = chapter;
			}
		}

		const RectF startButton{ 300.0 + ChapterSelectOffset.x - cameraX, 460.0 + ChapterSelectOffset.y, 200.0, 56.0 };
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
		const auto& data = getData().game;
		const SaveData& saveData = data.getSaveData();
		const int32 highestChapter = saveData.highestChapter;

		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(ColorF{ 0.08, 0.08, 0.10 });
		{
			const Transformer2D cameraTransformer{ Mat3x2::Translate(-cameraX, 0.0) };

			FontAsset(U"Big")(U"I wanna Siv3D").drawAt(400, 150, Palette::White);

			for (int32 chapter = 1; chapter <= 6; ++chapter) {
				const RectF button = chapterButtonRect(chapter);
				const bool unlocked = (chapter <= highestChapter);
				const bool selected = (chapter == selectedChapter);
				const bool hovered = unlocked && button.movedBy(-cameraX, 0.0).mouseOver();
				const ColorF fill = (!unlocked)
					? ColorF{ 0.10, 0.10, 0.13 }
					: (selected
					? ColorF{ 0.25, 0.65, 1.0 }
					: (hovered ? ColorF{ 0.25, 0.25, 0.30 } : ColorF{ 0.16, 0.16, 0.20 }));
				const ColorF frame = (!unlocked)
					? ColorF{ 0.24, 0.24, 0.30 }
					: (selected ? ColorF{ 0.75, 0.90, 1.0 } : ColorF{ 0.45, 0.45, 0.52 });
				const ColorF textColor = unlocked ? ColorF{ 1.0 } : ColorF{ 0.42, 0.44, 0.50 };

				button.rounded(6.0).draw(fill);
				button.rounded(6.0).drawFrame(2.0, frame);
				FontAsset(U"Button")(U"Ch " + Format(chapter)).drawAt(button.center(), textColor);
			}

			const RectF startButton{ 300.0 + ChapterSelectOffset.x, 460.0 + ChapterSelectOffset.y, 200.0, 56.0 };
			const ColorF startFill = startButton.movedBy(-cameraX, 0.0).mouseOver()
				? ColorF{ 0.95, 0.95, 1.0 }
				: ColorF{ 0.78, 0.82, 0.92 };
			startButton.rounded(6.0).draw(startFill);
			startButton.rounded(6.0).drawFrame(2.0, ColorF{ 0.25, 0.30, 0.42 });
			FontAsset(U"Button")(U"Start Chapter " + Format(selectedChapter)).drawAt(startButton.center(), ColorF{ 0.06, 0.07, 0.10 });

			FontAsset(U"Button")(U"Shift also starts the selected chapter").drawAt(400 + ChapterSelectOffset.x, 550 + ChapterSelectOffset.y, ColorF{ 0.60, 0.64, 0.72 });
		}

		const Vec2 saveTextPos{ 28.0, Global::windowHeight - 112.0 };
		const double lineHeight = 28.0;
		const String enduranceText =
			formatEnduranceTime(saveData.highestEnduranceSec)
			+ U" / "
			+ formatEnduranceTime(data.getEnduranceLengthSec());
		const ColorF saveTextColor{ 0.72, 0.76, 0.84 };

		FontAsset(U"Button")(U"Death : " + Format(saveData.deathCount)).draw(saveTextPos, saveTextColor);
		FontAsset(U"Button")(U"Play Time : " + formatPlayTime(saveData.playTimeSec)).draw(saveTextPos + Vec2{ 0.0, lineHeight }, saveTextColor);
		FontAsset(U"Button")(U"Best : " + enduranceText).draw(saveTextPos + Vec2{ 0.0, lineHeight * 2.0 }, saveTextColor);

		drawArrowButton(-1, cameraPage > CameraMinPage || isCameraMoving);
		drawArrowButton(1, cameraPage < CameraMaxPage || isCameraMoving);
	}
}
