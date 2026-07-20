#include "Scene.h"

namespace Iwanna {
	namespace {
		constexpr Vec2 ChapterSelectOffset = { 110.0, 30.0 };
		constexpr int32 ChapterButtonsPerRow = 5;
		constexpr double ChapterButtonBaseX = 190.0;
		constexpr double ChapterButtonBaseY = 330.0;
		constexpr double ChapterButtonSpacingX = 84.0;
		constexpr double ChapterButtonSpacingY = 56.0;
		constexpr int32 CameraMinPage = -1;
		constexpr int32 CameraMaxPage = 1;
		constexpr double CameraMoveDurationSec = 0.45;
		constexpr double LeftPageX = -Global::windowWidth;
		constexpr double RightPageX = Global::windowWidth;

		struct AchievementViewData {
			String title;
			String description;
			String textureName;
		};

		const std::array<AchievementViewData, SaveData::AchievementCount> AchievementViews{ {
			{ U"First Start", U"Started the endurance for the first time.", U"achive_start" },
			{ U"Chapter 1", U"Passed chapter 1.", U"achive_chapter1" },
			{ U"Chapter 2", U"Passed chapter 2.", U"achive_chapter2" },
			{ U"Chapter 3", U"Passed chapter 3.", U"achive_chapter3" },
			{ U"Chapter 4", U"Passed chapter 4.", U"achive_chapter4" },
			{ U"Chapter 5", U"Passed chapter 5.", U"" },
			{ U"Chapter 6", U"Passed chapter 6.", U"" },
		} };

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

		RectF achievementIconRect(int32 index) {
			constexpr int32 iconsPerRow = 9;
			const int32 column = index % iconsPerRow;
			const int32 row = index / iconsPerRow;
			return RectF{
				LeftPageX + 86.0 + column * 78.0,
				388.0 + row * 58.0,
				50.0,
				50.0,
			};
		}

		RectF tutorialButtonRect() {
			return RectF{ RightPageX + 260.0, 520.0, 280.0, 58.0 };
		}

		RectF difficultyButtonRect(int32 difficulty) {
			return RectF{ RightPageX + 120.0 + (difficulty - 1) * 160.0, 425.0, 132.0, 46.0 };
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

		void drawLeftPageRecord(const SaveData& saveData) {
			const Vec2 pageOffset{ LeftPageX, 0.0 };
			FontAsset(U"Big")(U"Record").draw(pageOffset + Vec2{ 84.0, 20.0 }, Palette::White);

			const RectF table{ LeftPageX + 84.0, 112.0, 630.0, 58.0 };
			const double firstColumnWidth = 102.0;
			const double dataColumnWidth = (table.w - firstColumnWidth) / SaveData::ChapterCount;
			table.drawFrame(1.5, Palette::White);
			Line{ table.x, table.y + 29.0, table.x + table.w, table.y + 29.0 }.draw(1.5, Palette::White);
			Line{ table.x + firstColumnWidth, table.y, table.x + firstColumnWidth, table.y + table.h }.draw(1.5, Palette::White);

			FontAsset(U"Button")(U"Chapter").drawAt(Vec2{ table.x + firstColumnWidth * 0.5, table.y + 14.5 }, Palette::White);
			FontAsset(U"Button")(U"Death").drawAt(Vec2{ table.x + firstColumnWidth * 0.5, table.y + 43.5 }, Palette::White);

			for (int32 chapter = 1; chapter <= SaveData::ChapterCount; ++chapter) {
				const double x = table.x + firstColumnWidth + (chapter - 1) * dataColumnWidth;
				Line{ x, table.y, x, table.y + table.h }.draw(1.0, ColorF{ 0.9 });
				FontAsset(U"Button")(Format(chapter)).drawAt(Vec2{ x + dataColumnWidth * 0.5, table.y + 14.5 }, Palette::White);
				FontAsset(U"Button")(Format(saveData.chapterDeathCounts[chapter - 1])).drawAt(Vec2{ x + dataColumnWidth * 0.5, table.y + 43.5 }, Palette::White);
			}
		}

		void drawLeftPageAchievements(const SaveData& saveData, int32 selectedAchievement, double cameraX) {
			const Vec2 pageOffset{ LeftPageX, 0.0 };
			Line{ pageOffset + Vec2{ 0.0, 304 }, pageOffset + Vec2{ 800.0, 304.0 } }.draw(1.5, Palette::White);
			FontAsset(U"Big")(U"Achievement").draw(pageOffset + Vec2{ 84.0, 300.0 }, Palette::White);

			for (int32 index = 0; index < SaveData::AchievementCount; ++index) {
				const RectF icon = achievementIconRect(index);
				const bool unlocked = !saveData.achievementUnlockedAt[index].isEmpty();
				const bool selected = (index == selectedAchievement);
				const bool hovered = icon.movedBy(-cameraX, 0.0).mouseOver();
				const ColorF fill = unlocked
					? (hovered ? ColorF{ 0.33, 0.39, 0.52 } : ColorF{ 0.20, 0.25, 0.34 })
					: ColorF{ 0.08, 0.09, 0.12 };
				const ColorF frame = selected ? ColorF{ 1.0, 0.82, 0.38 } : ColorF{ 0.92 };

				icon.draw(fill);
				const String textureName = unlocked ? AchievementViews[index].textureName : U"achive_locked";
				if (!textureName.isEmpty()) {
					TextureAsset(textureName).resized(icon.w, icon.h).draw(icon.pos);
				}
				else {
					FontAsset(U"Button")(Format(index + 1)).drawAt(icon.center(), ColorF{ 1.0 });
				}
				icon.drawFrame(selected ? 3.0 : 2.0, frame);
			}

			const RectF detailBand{ LeftPageX, 540.0, Global::windowWidth, 168 };
			detailBand.draw(ColorF{ 0.96, 0.97, 1.0 });

			const auto& achievement = AchievementViews[selectedAchievement];
			const String unlockedAt = saveData.achievementUnlockedAt[selectedAchievement].isEmpty()
				? U"Not unlocked"
				: saveData.achievementUnlockedAt[selectedAchievement];
			const ColorF textColor{ 0.08, 0.09, 0.12 };

			const double achivementDescOffsetY = 85;

			FontAsset(U"Button")(achievement.description).draw(pageOffset + Vec2{ 26.0, 456.0 + achivementDescOffsetY }, textColor);
			FontAsset(U"Button")(U"Unlocked : " + unlockedAt).draw(pageOffset + Vec2{ 26.0, 488.0 + achivementDescOffsetY }, textColor);
		}

		void drawRightPageOptions(MainGame& game, double cameraX) {
			const double pageX = RightPageX - cameraX;
			FontAsset(U"Big")(U"Option").draw(Vec2{ pageX + 84.0, 36.0 }, Palette::White);
			FontAsset(U"Button")(U"Volume").draw(Vec2{ pageX + 128.0, 170.0 }, ColorF{ 0.78, 0.82, 0.92 });

			double bgmVolume = game.getSaveData().bgmVolume;
			double seVolume = game.getSaveData().seVolume;

			SimpleGUI::Slider(U"BGM", bgmVolume, 0.0, 1.0, Vec2{ pageX + 120.0, 230.0 }, 88.0, 360.0);
			SimpleGUI::Slider(U"SE", seVolume, 0.0, 1.0, Vec2{ pageX + 120.0, 300.0 }, 88.0, 360.0);

			FontAsset(U"Button")(ToFixed(bgmVolume * 100.0, 0) + U"%").draw(Vec2{ pageX + 590.0, 229.0 }, Palette::White);
			FontAsset(U"Button")(ToFixed(seVolume * 100.0, 0) + U"%").draw(Vec2{ pageX + 590.0, 299.0 }, Palette::White);

			FontAsset(U"Button")(U"Difficulty").draw(Vec2{ pageX + 128.0, 374.0 }, ColorF{ 0.78, 0.82, 0.92 });
			const bool canChangeDifficulty = game.canChangeDifficulty();
			for (int32 difficulty = 1; difficulty <= 2; ++difficulty) {
				const RectF button = difficultyButtonRect(difficulty).movedBy(-cameraX, 0.0);
				const Global::Difficulty difficultyValue = (difficulty == 1)
					? Global::Difficulty::Easy
					: Global::Difficulty::Medium;
				const bool selected = (game.getSaveData().difficulty == difficultyValue);
				const bool hovered = canChangeDifficulty && button.mouseOver();
				const ColorF fill = selected
					? ColorF{ 0.25, 0.65, 1.0 }
					: (canChangeDifficulty
						? (hovered ? ColorF{ 0.25, 0.25, 0.30 } : ColorF{ 0.16, 0.16, 0.20 })
						: ColorF{ 0.10, 0.10, 0.13 });
				const ColorF frame = selected ? ColorF{ 0.75, 0.90, 1.0 } : ColorF{ 0.45, 0.45, 0.52 };
				const ColorF text = canChangeDifficulty || selected ? ColorF{ 1.0 } : ColorF{ 0.42, 0.44, 0.50 };
				const String label = (difficulty == 1) ? U"Easy" : U"Medium";

				button.rounded(6.0).draw(fill);
				button.rounded(6.0).drawFrame(2.0, frame);
				FontAsset(U"Button")(label).drawAt(button.center(), text);
			}
			if (!canChangeDifficulty) {
				FontAsset(U"Button")(U"Locked after starting avoidance").draw(Vec2{ pageX + 430.0, 435.0 }, ColorF{ 0.60, 0.64, 0.72 });
			}

			const RectF tutorialButton = tutorialButtonRect().movedBy(-cameraX, 0.0);
			const bool hovered = tutorialButton.mouseOver();
			tutorialButton.rounded(6.0).draw(hovered ? ColorF{ 0.95, 0.95, 1.0 } : ColorF{ 0.78, 0.82, 0.92 });
			tutorialButton.rounded(6.0).drawFrame(2.0, ColorF{ 0.25, 0.30, 0.42 });
			FontAsset(U"Button")(U"Tutorial").drawAt(tutorialButton.center(), ColorF{ 0.06, 0.07, 0.10 });

			if (0.001 < Abs(bgmVolume - game.getSaveData().bgmVolume)) {
				game.setBgmVolume(bgmVolume);
			}
			if (0.001 < Abs(seVolume - game.getSaveData().seVolume)) {
				game.setSeVolume(seVolume);
			}
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
		if (data.canStartAvoidance()) {
			showDifficultyMessage = false;
		}
		updateCameraMove();

		if (arrowButtonRect(-1).leftClicked()) {
			requestCameraMove(-1);
		}
		if (arrowButtonRect(1).leftClicked()) {
			requestCameraMove(1);
		}

		if (tutorialButtonRect().movedBy(-cameraX, 0.0).leftClicked()) {
			data.startTutorial();
			changeScene(SceneType::IN_GAME, 0.0s);
			return;
		}

		if (data.canChangeDifficulty()) {
			for (int32 difficulty = 1; difficulty <= 2; ++difficulty) {
				if (difficultyButtonRect(difficulty).movedBy(-cameraX, 0.0).leftClicked()) {
					data.setDifficulty((difficulty == 1) ? Global::Difficulty::Easy : Global::Difficulty::Medium);
					showDifficultyMessage = false;
				}
			}
		}

		for (int32 chapter = 1; chapter <= 6; ++chapter) {
			const RectF button = chapterButtonRect(chapter).movedBy(-cameraX, 0.0);
			const bool unlocked = (chapter <= highestChapter);

			if (unlocked && button.leftClicked()) {
				selectedChapter = chapter;
			}
		}

		for (int32 index = 0; index < SaveData::AchievementCount; ++index) {
			const RectF icon = achievementIconRect(index).movedBy(-cameraX, 0.0);
			if (icon.leftClicked()) {
				selectedAchievement = index;
			}
		}

		const RectF startButton{ 300.0 + ChapterSelectOffset.x - cameraX, 460.0 + ChapterSelectOffset.y, 200.0, 56.0 };
		if (startButton.leftClicked()) {
			if (!data.canStartAvoidance()) {
				showDifficultyMessage = true;
				return;
			}
			data.startGame(selectedChapter);
			changeScene(SceneType::IN_GAME, 0.0s);
			return;
		}

		if (cameraPage == 0 && Global::inputStart.down()) {
			if (!data.canStartAvoidance()) {
				showDifficultyMessage = true;
				return;
			}
			data.startGame(selectedChapter);
			changeScene(SceneType::IN_GAME, 0.0s);
		}
	}

	void StartMenu::draw() const {
		auto& mutableGame = const_cast<MainGame&>(getData().game);
		const auto& data = mutableGame;
		const SaveData& saveData = data.getSaveData();
		const int32 highestChapter = saveData.highestChapter;

		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(ColorF{ 0.08, 0.08, 0.10 });
		{
			const Transformer2D cameraTransformer{ Mat3x2::Translate(-cameraX, 0.0) };

			drawLeftPageRecord(saveData);
			drawLeftPageAchievements(saveData, selectedAchievement, cameraX);

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
			if (showDifficultyMessage) {
				FontAsset(U"Button")(U"Please select difficulty in option screen").drawAt(400, 300, ColorF{ 1.0, 0.82, 0.38 });
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
		}

		drawRightPageOptions(mutableGame, cameraX);

		drawArrowButton(-1, cameraPage > CameraMinPage || isCameraMoving);
		drawArrowButton(1, cameraPage < CameraMaxPage || isCameraMoving);
	}
}
