#include "AvoidanceManager.h"

namespace {
	struct Chapter4Timing {
		int32 firstPreview = 14;
		int32 firstRing = 55;
		int32 firstSpokes = 105;
		int32 firstCleanup = 125;
		int32 secondPreview = 135;
		int32 symbolWarning = 155;
		int32 secondCleanup = 175;
		int32 coloredPerimeter = 180;
		int32 giantSweep = 225;
		int32 convergingRing = 275;
	};

	struct Chapter4ScreenEffectSettings {
		int32 firstFadeIn = 8;
		int32 firstFadeInDuration = 8;
		int32 firstFlash = 55;
		int32 flashHoldDuration = 2;
		int32 flashFadeOutDuration = 8;
		int32 firstFadeOut = 125;
		int32 firstFadeOutDuration = 8;
		int32 secondFadeIn = 130;
		int32 secondFadeInDuration = 5;
		int32 secondFlash = 175;
		int32 secondFadeOut = 185;
		int32 secondFadeOutDuration = 10;
	};

	struct Chapter4RingSettings {
		Vec2 center = Vec2{ 400,304 };
		int32 previewCount = 3;
		double previewSpacing = 72.0;
		int32 outerCount = 12;
		double outerRadius = 235.0;
		double outerScale = 4.0;
		int32 spokeCount = 3;
		int32 spokeAppleCount = 8;
		double spokeScale = 1.2;
		double anchorScale = 4.5;
		int32 appearanceTime = 14;
	};

	struct Chapter4FieldSettings {
		int32 perimeterCountPerSide = 18;
		double perimeterSpeed = 0.7;
		double perimeterScale = 0.85;
		int32 sweepRows = 4;
		int32 sweepApplesPerRow = 11;
		double sweepStartY = 150.0;
		double sweepRowSpacing = 96.0;
		double sweepAppleSpacing = 72.0;
		double sweepSpeed = 50.0;
		double sweepScale = 4.0;
		int32 convergeCount = 48;
		double convergeStartRadius = 470.0;
		double convergeEndRadius = 215.0;
		int32 convergeTime = 65;
		double convergeAngularSpeed = 0.8;
		double convergeScale = 1.15;
	};

	const Chapter4Timing timing{};
	const Chapter4ScreenEffectSettings screenEffectSettings{};
	const Chapter4RingSettings ringSettings{};
	const Chapter4FieldSettings fieldSettings{};

	Array<std::shared_ptr<Iwanna::Cherry>> firstAttackCherries;
	Array<std::shared_ptr<Iwanna::Cherry>> secondWarningCherries;

	void removeWithEffect(Array<std::shared_ptr<Iwanna::Cherry>>& cherries, int32 duration = 10) {
		for (const auto& cherry : cherries) {
			if (cherry && !cherry->isDelete) {
				cherry->setEffect(Iwanna::CherryEffect::ScaleOut, duration);
			}
		}
		cherries.clear();
	}
}

namespace Iwanna {
	// step : 1820 - 2299
	void AvoidanceManager::chapter4() {
		Timeline timeline(previousStep, step);
		const int32 chapterStart = Global::startStep_Chapter4;

		const auto createCherryWithSettings = [&](const Vec2& pos, const Cherry::Settings& settings) {
			auto cherry = std::make_shared<Cherry>();
			cherry->pos = pos;
			cherry->applySettings(settings);
			createCherry(cherry);
			return cherry;
		};

		const auto createPreview = [&](Array<std::shared_ptr<Cherry>>& stock) {
			for (int32 i = 0; i < ringSettings.previewCount; ++i) {
				const double offset = (i - (ringSettings.previewCount - 1) / 2.0)
					* ringSettings.previewSpacing;
				stock << createCherryWithSettings(
					ringSettings.center + Vec2{ offset,0 },
					Cherry::Settings{
						.textureName = U"sprCherryWhite",
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.depth = DrawDepth::Cherry,
						.appearanceEffect = CherryEffect::FadeIn,
						.appearanceDuration = ringSettings.appearanceTime,
					});
			}
		};

		timeline.at(chapterStart, [&] {
			firstAttackCherries.clear();
			secondWarningCherries.clear();
			resetScreenEffect();
		});

		timeline.at(chapterStart + screenEffectSettings.firstFadeIn, [&] {
			fadeScreenIn(screenEffectSettings.firstFadeInDuration);
		});

		timeline.at(chapterStart + screenEffectSettings.firstFlash, [&] {
			flashScreen(
				screenEffectSettings.flashHoldDuration,
				screenEffectSettings.flashFadeOutDuration);
		});

		timeline.at(chapterStart + screenEffectSettings.firstFadeOut, [&] {
			fadeScreenOut(screenEffectSettings.firstFadeOutDuration);
		});

		timeline.at(chapterStart + screenEffectSettings.secondFadeIn, [&] {
			fadeScreenIn(screenEffectSettings.secondFadeInDuration);
		});

		timeline.at(chapterStart + screenEffectSettings.secondFlash, [&] {
			flashScreen(
				screenEffectSettings.flashHoldDuration,
				screenEffectSettings.flashFadeOutDuration);
		});

		timeline.at(chapterStart + screenEffectSettings.secondFadeOut, [&] {
			fadeScreenOut(screenEffectSettings.secondFadeOutDuration);
		});

		timeline.at(chapterStart + timing.firstPreview, [&] {
			createPreview(firstAttackCherries);
		});

		timeline.at(chapterStart + timing.firstRing, [&] {
			removeWithEffect(firstAttackCherries, 8);

			for (int32 i = 0; i < ringSettings.outerCount; ++i) {
				const double angle = -90.0 + 360.0 * i / ringSettings.outerCount;
				const Vec2 pos = ringSettings.center + Vec2{
					ringSettings.outerRadius * Math::Cos(Math::ToRadians(angle)),
					ringSettings.outerRadius * Math::Sin(Math::ToRadians(angle))
				};
				firstAttackCherries << createCherryWithSettings(
					pos,
					Cherry::Settings{
						.textureName = U"sprCherryWhite",
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.depth = DrawDepth::Cherry,
						.scale = ringSettings.outerScale,
						.appearanceEffect = CherryEffect::ScaleIn,
						.appearanceDuration = ringSettings.appearanceTime,
					});
			}
		});

		timeline.at(chapterStart + timing.firstSpokes, [&] {
			for (int32 spoke = 0; spoke < ringSettings.spokeCount; ++spoke) {
				const double angle = -90.0 + 360.0 * spoke / ringSettings.spokeCount;
				const Vec2 unit{
					Math::Cos(Math::ToRadians(angle)),
					Math::Sin(Math::ToRadians(angle))
				};

				firstAttackCherries << createCherryWithSettings(
					ringSettings.center + unit * ringSettings.outerRadius,
					Cherry::Settings{
						.textureName = U"sprCherryAllWhite",
						.color = ColorF{ 1.0,0.08,0.08 },
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.depth = DrawDepth::Cherry + 1.0,
						.scale = ringSettings.anchorScale,
						.appearanceEffect = CherryEffect::ScaleIn,
						.appearanceDuration = 8,
					});

				for (int32 i = 1; i <= ringSettings.spokeAppleCount; ++i) {
					const double distance = ringSettings.outerRadius * i
						/ (ringSettings.spokeAppleCount + 1.0);
					firstAttackCherries << createCherryWithSettings(
						ringSettings.center + unit * distance,
						Cherry::Settings{
							.textureName = U"sprCherryAllWhite",
							.color = ColorF{ 0.68,0.68,0.58 },
							.canDeleteOutOfScreen = false,
							.canPlayerKill = true,
							.depth = DrawDepth::Cherry + 1.0,
							.scale = ringSettings.spokeScale,
							.appearanceEffect = CherryEffect::ScaleIn,
							.appearanceDuration = 6,
						});
				}
			}
		});

		timeline.at(chapterStart + timing.firstCleanup, [&] {
			removeWithEffect(firstAttackCherries, 8);
		});

		timeline.at(chapterStart + timing.secondPreview, [&] {
			createPreview(secondWarningCherries);
		});

		timeline.at(chapterStart + timing.symbolWarning, [&] {
			removeWithEffect(secondWarningCherries, 6);
			const Vec2 circleCenter = ringSettings.center + Vec2{ -55,-25 };
			const Vec2 crossCenter = ringSettings.center + Vec2{ 55,25 };

			for (int32 i = 0; i < 12; ++i) {
				const double angle = 360.0 * i / 12.0;
				secondWarningCherries << createCherryWithSettings(
					circleCenter + Vec2{
						22.0 * Math::Cos(Math::ToRadians(angle)),
						22.0 * Math::Sin(Math::ToRadians(angle))
					},
					Cherry::Settings{
						.textureName = U"sprCherryAllWhite",
						.canDeleteOutOfScreen = false,
						.canPlayerKill = false,
						.scale = 0.45,
						.appearanceEffect = CherryEffect::FadeIn,
						.appearanceDuration = 6,
					});
			}

			for (int32 i = -3; i <= 3; ++i) {
				for (const int32 sign : { -1,1 }) {
					secondWarningCherries << createCherryWithSettings(
						crossCenter + Vec2{ i * 8.0, sign * i * 8.0 },
						Cherry::Settings{
							.textureName = U"sprCherryAllWhite",
							.canDeleteOutOfScreen = false,
							.canPlayerKill = false,
							.scale = 0.45,
							.appearanceEffect = CherryEffect::FadeIn,
							.appearanceDuration = 6,
						});
				}
			}
		});

		timeline.at(chapterStart + timing.secondCleanup, [&] {
			removeWithEffect(secondWarningCherries, 6);
		});

		timeline.at(chapterStart + timing.coloredPerimeter, [&] {
			const int32 total = fieldSettings.perimeterCountPerSide * 4;
			for (int32 i = 0; i < total; ++i) {
				const int32 side = i / fieldSettings.perimeterCountPerSide;
				const double sideT = (i % fieldSettings.perimeterCountPerSide + 0.5)
					/ fieldSettings.perimeterCountPerSide;
				Vec2 pos;

				switch (side) {
				case 0: pos = Vec2{ sideT * Global::windowWidth,8 }; break;
				case 1: pos = Vec2{ Global::windowWidth - 8,sideT * Global::windowHeight }; break;
				case 2: pos = Vec2{ (1.0 - sideT) * Global::windowWidth,Global::windowHeight - 8 }; break;
				default: pos = Vec2{ 8,(1.0 - sideT) * Global::windowHeight }; break;
				}

				auto cherry = createCherryWithSettings(
					pos,
					Cherry::Settings{
						.textureName = U"sprCherryAllWhite",
						.color = ColorF{ HSV(360.0 * i / total,0.9,1.0) },
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.scale = fieldSettings.perimeterScale,
						.appearanceEffect = CherryEffect::FadeIn,
						.appearanceDuration = 10,
					});
				const Vec2 diff = ringSettings.center - pos;
				cherry->dir = Math::ToDegrees(Atan2(-diff.y, diff.x));
				cherry->speed = fieldSettings.perimeterSpeed;
			}
		});

		timeline.at(chapterStart + timing.giantSweep, [&] {
			for (int32 row = 0; row < fieldSettings.sweepRows; ++row) {
				for (int32 i = 0; i < fieldSettings.sweepApplesPerRow; ++i) {
					auto cherry = createCherryWithSettings(
						Vec2{
							Global::windowWidth + 80.0 + i * fieldSettings.sweepAppleSpacing,
							fieldSettings.sweepStartY + row * fieldSettings.sweepRowSpacing
						},
						Cherry::Settings{
							.textureName = U"sprCherryAllWhite",
							.color = ((i + row) % 2 == 0)
								? ColorF{ 0.9,0.9,0.9 }
								: ColorF{ 0.28,0.28,0.28 },
							.behavior = [](Cherry& self, int32) {
								if (self.pos.x < -200.0) {
									self.isDelete = true;
								}
							},
							.canDeleteOutOfScreen = false,
							.canPlayerKill = true,
							.depth = DrawDepth::Cherry + 2.0,
							.scale = fieldSettings.sweepScale,
						});
					cherry->dir = 180.0;
					cherry->speed = fieldSettings.sweepSpeed;
				}
			}
		});

		timeline.at(chapterStart + timing.convergingRing, [&] {
			for (int32 i = 0; i < fieldSettings.convergeCount; ++i) {
				const double baseAngle = 360.0 * i / fieldSettings.convergeCount;
				const Vec2 startPos = ringSettings.center + Vec2{
					fieldSettings.convergeStartRadius * Math::Cos(Math::ToRadians(baseAngle)),
					fieldSettings.convergeStartRadius * Math::Sin(Math::ToRadians(baseAngle))
				};

				createCherryWithSettings(
					startPos,
					Cherry::Settings{
						.textureName = U"sprCherryAllWhite",
						.color = ColorF{ HSV(baseAngle,0.9,1.0) },
						.behavior = [baseAngle](Cherry& self, int32 age) {
							const double t = Clamp(
								age / static_cast<double>(fieldSettings.convergeTime),
								0.0,
								1.0);
							const double eased = t * t * (3.0 - 2.0 * t);
							const double radius = fieldSettings.convergeStartRadius
								+ (fieldSettings.convergeEndRadius - fieldSettings.convergeStartRadius) * eased;
							const double angle = baseAngle + age * fieldSettings.convergeAngularSpeed;
							self.pos = ringSettings.center + Vec2{
								radius * Math::Cos(Math::ToRadians(angle)),
								radius * Math::Sin(Math::ToRadians(angle))
							};
						},
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.depth = DrawDepth::Cherry + 1.0,
						.scale = fieldSettings.convergeScale,
						.appearanceEffect = CherryEffect::FadeIn,
						.appearanceDuration = 12,
					});
			}
		});
	}
}
