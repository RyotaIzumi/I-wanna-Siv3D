#include "AvoidanceManager.h"
#include <array>
#include <cmath>
#include <utility>

namespace {
	struct Chapter1GridSettings {
		String frameTextureName = U"sprCherryWhite";
		String fillTextureName = U"sprCherryAllWhite";
		ColorF frameColor = Palette::Gray;
		ColorF fillColor = Palette::White;
		double outerLineAlpha = 1.0;
		double innerLineAlpha = 0.38;
		double fillAlpha = 1.0;
		double lineSpacing = 12.0;
		double fillSpacing = 16.0;
		double lineScale = 0.72;
		double fillScale = 1.1;
		double outerDepth = Iwanna::DrawDepth::Cherry + 3.0;
		double innerDepth = Iwanna::DrawDepth::Cherry + 2.0;
		double fillDepth = Iwanna::DrawDepth::Cherry + 1.0;
	};

	const Chapter1GridSettings chapter1GridSettings{};

	struct Chapter1SniperSightSettings {
		String textureName = U"sprCherryAllWhite";
		double radius = 58.0;
		int32 circleCherryCount = 50;
		double crossSpacing = 5.0;
		double crossExtend = 10.0;
		double outerScale = 0.5;
		double crossScale = 0.2;
		double centerScale = 0.55;
		double rotationSpeed = 0.04;
		int32 rotationStartStep = 15;
		int32 appearanceStep = 15;
		int32 firstActionStep = 15;
		int32 actionIntervalStep = 12;
		int32 pulseDurationStep = 10;
		double pulseScale = 1.28;
		int32 sightMoveDurationStep = 15;
		double screenShakeAmplitude = 6.0;
		int32 screenShakeDurationStep = 18;
		double screenShakeFrequency = 0.85;
		double panelFlySpeed = 11.0;
		double panelLiftSpeed = 4.0;
		double panelGravity = 0.98;
		double panelBottomDeleteMargin = 48.0;
		double panelMinRotationSpeed = 0.08;
		double panelMaxRotationSpeed = 0.15;
		ColorF panelHitColor = ColorF{ 1.0, 0.0, 0.0, 1.0 };
		int32 panelHitColorFadeStep = 10;
		ColorF outerColor = ColorF{ 0.3, 0.95 };
		ColorF crossColor = ColorF{ 0.3, 0.95 };
		int32 sightCount = 3;
		Array<int32> initialCandidateSquareIndices = { 0, 1, 2, 3, 5, 6, 7, 8 };
		Array<int32> moveCandidateSquareIndices = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
		Array<ColorF> centerColors = {
			ColorF{ 1.0, 0.0, 0.0, 1.0 },
			ColorF{ 0.0, 0.25, 1.0, 1.0 },
			ColorF{ 0.0, 0.85, 0.2, 1.0 },
		};
	};

	const Chapter1SniperSightSettings chapter1SniperSightSettings{};

	struct Chapter1OpeningFadeSettings {
		int32 startStep = 10;
		int32 durationStep = 10;
		ColorF color = ColorF{ 0.0, 0.0, 0.0, 1.0 };
	};

	const Chapter1OpeningFadeSettings chapter1OpeningFadeSettings{};

	struct Chapter1RotatingRingSettings {
		String textureName = U"sprCherryWhite";
		ColorF color = ColorF{ 0.95, 0.95, 1.0, 1.0 };
		Vec2 center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		int32 ringCount = 3;
		int32 cherryCount = 20;
		int32 gapInterval = 100;
		double minRadius = 326.0;
		double radiusSpacing = 80.0;
		double scale = 5.0;
		double baseRotationSpeed = 0.006;
		double rotationSpeedStep = 0.002;
		double depth = Iwanna::DrawDepth::Cherry + 0.5;
		bool canPlayerKill = true;
	};

	const Chapter1RotatingRingSettings chapter1RotatingRingSettings{};

	struct Chapter1RotatingRingController {
		Vec2 center = Vec2{ 0,0 };
		double rotationSpeed = 0.0;
	};

	struct Chapter1SquareController {
		int32 index = 0;
		Vec2 center = Vec2{ 0,0 };
		Vec2 offset = Vec2{ 0,0 };
		Vec2 flyVelocity = Vec2{ 0,0 };
		double flyGravity = 0.0;
		double rotationSpeed = 0.0;
		int32 flyStartStep = -1;
		int32 attackStep = -1;
		int32 hitColorStartStep = -1;

		void resetState() {
			offset = Vec2{ 0,0 };
			flyVelocity = Vec2{ 0,0 };
			flyGravity = 0.0;
			rotationSpeed = 0.0;
			flyStartStep = -1;
			attackStep = -1;
			hitColorStartStep = -1;
		}
	};

	Array<std::shared_ptr<Chapter1SquareController>> chapter1SquareControllers;
	Array<std::shared_ptr<Chapter1RotatingRingController>> chapter1RotatingRingControllers;
	Array<int32> chapter1SniperSightSquareIndices;
	Array<int32> chapter1SightInitialSquareIndices;
	Array<int32> chapter1SightCurrentSquareIndices;
	Array<int32> chapter1SightMoveStartSquareIndices;
	Array<int32> chapter1SightMoveTargetSquareIndices;
	Array<int32> chapter1SightMoveStartSteps;
	Array<int32> chapter1SightLastActionSteps;
	Array<int32> chapter1SightActionCounts;
	Array<int32> chapter1SightRingTargetCherryIndices;
	Array<int32> chapter1SightRingMoveStartSteps;
	Array<Vec2> chapter1SightRingMoveStartCenters;
	Array<int32> chapter1UsedSquareIndices;

	using GridPoints = std::array<std::array<Vec2, 4>, 4>;

	const GridPoints chapter1GridPoints = { {
		{ Vec2{ 184, 84 }, Vec2{ 328, 84 }, Vec2{ 472, 84 }, Vec2{ 616, 84 } },
		{ Vec2{ 184, 228 }, Vec2{ 328, 228 }, Vec2{ 472, 228 }, Vec2{ 616, 228 } },
		{ Vec2{ 184, 372 }, Vec2{ 328, 372 }, Vec2{ 472, 372 }, Vec2{ 616, 372 } },
		{ Vec2{ 184, 516 }, Vec2{ 328, 516 }, Vec2{ 472, 516 }, Vec2{ 616, 516 } },
	} };

	ColorF interpolateColor(const ColorF& start, const ColorF& end, double t) {
		const double rate = Clamp(t, 0.0, 1.0);
		return ColorF{
			start.r + (end.r - start.r) * rate,
			start.g + (end.g - start.g) * rate,
			start.b + (end.b - start.b) * rate,
			start.a + (end.a - start.a) * rate,
		};
	}

	Iwanna::Cherry::Behavior makeChapter1SquareBehavior(
		const Vec2& home,
		const std::shared_ptr<Chapter1SquareController>& controller) {

		return [home, controller](Iwanna::Cherry& self, int32) {
			Vec2 offset = controller->offset;
			Vec2 panelLocalPos = home - controller->center;
			double panelAngle = 0.0;
			if (0 <= controller->flyStartStep && controller->flyStartStep <= self.getAge()) {
				const double flyAge = static_cast<double>(self.getAge() - controller->flyStartStep);
				offset += controller->flyVelocity * flyAge
					+ Vec2{ 0.0, 0.5 * controller->flyGravity * flyAge * flyAge };
				panelAngle = controller->rotationSpeed * flyAge;
				const double c = Math::Cos(panelAngle);
				const double s = Math::Sin(panelAngle);
				panelLocalPos = Vec2{
					panelLocalPos.x * c - panelLocalPos.y * s,
					panelLocalPos.x * s + panelLocalPos.y * c,
				};
			}

			self.pos = controller->center + panelLocalPos + offset;
			self.textureAngle = panelAngle;
			self.canPlayerKill = (self.getAge() == controller->attackStep);

			const auto& gridSettings = chapter1GridSettings;
			const auto& sightSettings = chapter1SniperSightSettings;
			if (0 <= controller->hitColorStartStep) {
				const int32 colorAge = self.getAge() - controller->hitColorStartStep;
				if (0 <= colorAge && colorAge <= sightSettings.panelHitColorFadeStep) {
					const double rate = colorAge / static_cast<double>(Max(sightSettings.panelHitColorFadeStep, 1));
					self.setColor(interpolateColor(sightSettings.panelHitColor, gridSettings.fillColor, rate));
				}
				else if (sightSettings.panelHitColorFadeStep < colorAge) {
					self.setColor(gridSettings.fillColor);
				}
			}

			if (0 <= controller->flyStartStep
				&& Global::windowHeight + chapter1SniperSightSettings.panelBottomDeleteMargin < self.pos.y) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter1RotatingRingBehavior(
		const Vec2& localOffset,
		const std::shared_ptr<Chapter1RotatingRingController>& controller) {

		return [localOffset, controller](Iwanna::Cherry& self, int32 age) {
			const double angle = controller->rotationSpeed * age;
			const double c = Math::Cos(angle);
			const double s = Math::Sin(angle);
			const Vec2 rotated{
				localOffset.x * c - localOffset.y * s,
				localOffset.x * s + localOffset.y * c,
			};

			self.pos = controller->center + rotated;
		};
	}

	Vec2 interpolateLine(const Vec2& start, const Vec2& end, double t) {
		return start + (end - start) * t;
	}

	Vec2 interpolateQuad(const Vec2& topLeft, const Vec2& topRight, const Vec2& bottomLeft, const Vec2& bottomRight, double u, double v) {
		const Vec2 top = interpolateLine(topLeft, topRight, u);
		const Vec2 bottom = interpolateLine(bottomLeft, bottomRight, u);
		return interpolateLine(top, bottom, v);
	}

	Vec2 getChapter1SquareCenter(int32 squareIndex) {
		const int32 clampedIndex = Clamp(squareIndex, 0, 8);
		const int32 row = clampedIndex / 3;
		const int32 column = clampedIndex % 3;

		return interpolateQuad(
			chapter1GridPoints[row][column],
			chapter1GridPoints[row][column + 1],
			chapter1GridPoints[row + 1][column],
			chapter1GridPoints[row + 1][column + 1],
			0.5,
			0.5);
	}

	void drawChapter1SniperSightAt(
		const Vec2& center,
		const ColorF& centerColor,
		double sizeRate,
		int32 localStep) {

		const auto& settings = chapter1SniperSightSettings;
		const double appearanceRate = Iwanna::applyEasing(
			Iwanna::EasingMoveType::EaseOut,
			localStep / static_cast<double>(Max(settings.appearanceStep, 1)));
		const double visibleRadius = settings.radius * appearanceRate * sizeRate;
		const double lineRadius = (settings.radius + settings.crossExtend) * appearanceRate * sizeRate;
		const auto texture = TextureAsset(settings.textureName);
		const int32 rotationStep = Max(localStep - settings.rotationStartStep, 0);
		const double rotation = rotationStep * settings.rotationSpeed;
		const auto rotate = [rotation](const Vec2& v) {
			const double c = Math::Cos(rotation);
			const double s = Math::Sin(rotation);
			return Vec2{
				v.x * c - v.y * s,
				v.x * s + v.y * c,
			};
		};

		for (int32 i = 0; i < settings.circleCherryCount; ++i) {
			const double angle = 6.283185307179586 * i / Max(settings.circleCherryCount, 1) + rotation;
			const Vec2 pos = center + Vec2{ Math::Cos(angle), Math::Sin(angle) } * visibleRadius;
			texture.scaled(settings.outerScale).drawAt(pos, settings.outerColor);
		}

		for (double offset = -(settings.radius + settings.crossExtend); offset <= settings.radius + settings.crossExtend; offset += settings.crossSpacing) {
			if (Abs(offset) <= settings.crossSpacing * 0.5) {
				continue;
			}

			const double visibleOffset = offset * appearanceRate * sizeRate;
			if (Abs(visibleOffset) <= lineRadius) {
				texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ visibleOffset, 0.0 }), settings.crossColor);
				texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ 0.0, visibleOffset }), settings.crossColor);
			}
		}

		texture.scaled(settings.centerScale * sizeRate).drawAt(center, centerColor);
	}

	int32 getChapter1SightActionStep(int32 actionIndex) {
		const auto& settings = chapter1SniperSightSettings;
		return settings.firstActionStep + actionIndex * settings.actionIntervalStep;
	}

	int32 getChapter1RotatingRingCherryCount(int32 ringIndex) {
		const auto& settings = chapter1RotatingRingSettings;
		const double radius = settings.minRadius + settings.radiusSpacing * ringIndex;
		return Max(
			static_cast<int32>(std::round(settings.cherryCount * radius / Max(settings.minRadius, 1.0))),
			settings.cherryCount);
	}

	Vec2 getChapter1RotatingRingCherryCenter(int32 ringIndex, int32 cherryIndex, int32 localStep) {
		const auto& settings = chapter1RotatingRingSettings;
		const int32 cherryCount = getChapter1RotatingRingCherryCount(ringIndex);
		const double radius = settings.minRadius + settings.radiusSpacing * ringIndex;
		const double direction = (ringIndex % 2 == 0) ? 1.0 : -1.0;
		const double rotationSpeed = direction * (settings.baseRotationSpeed + settings.rotationSpeedStep * ringIndex);
		const double angle = 6.283185307179586 * (cherryIndex % Max(cherryCount, 1)) / Max(cherryCount, 1)
			+ rotationSpeed * localStep;

		return settings.center + Vec2{ Math::Cos(angle) * radius, Math::Sin(angle) * radius };
	}

	double getChapter1SightPulseSizeRate(int32 sightIndex, int32 localStep) {
		const auto& settings = chapter1SniperSightSettings;
		if (sightIndex < 0
			|| chapter1SightLastActionSteps.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightLastActionSteps[sightIndex] < 0) {
			return 1.0;
		}

		const int32 pulseAge = localStep - chapter1SightLastActionSteps[sightIndex];
		if (pulseAge < 0 || settings.pulseDurationStep < pulseAge) {
			return 1.0;
		}

		const double t = pulseAge / static_cast<double>(Max(settings.pulseDurationStep, 1));
		const double pulseRate = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
		return 1.0 + (settings.pulseScale - 1.0) * pulseRate;
	}

	Vec2 getChapter1SightCenter(int32 sightIndex, int32 localStep) {
		if (0 <= sightIndex
			&& static_cast<size_t>(sightIndex) < chapter1SightRingTargetCherryIndices.size()
			&& static_cast<size_t>(sightIndex) < chapter1SightRingMoveStartSteps.size()
			&& static_cast<size_t>(sightIndex) < chapter1SightRingMoveStartCenters.size()
			&& 0 <= chapter1SightRingTargetCherryIndices[sightIndex]) {

			const auto& settings = chapter1SniperSightSettings;
			const int32 moveAge = localStep - chapter1SightRingMoveStartSteps[sightIndex];
			const Vec2 startCenter = chapter1SightRingMoveStartCenters[sightIndex];
			const Vec2 targetCenter = getChapter1RotatingRingCherryCenter(
				0,
				chapter1SightRingTargetCherryIndices[sightIndex],
				localStep);

			if (moveAge <= 0) {
				return startCenter;
			}

			const double t = moveAge / static_cast<double>(Max(settings.sightMoveDurationStep, 1));
			const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			return startCenter + (targetCenter - startCenter) * rate;
		}

		if (sightIndex < 0
			|| chapter1SightCurrentSquareIndices.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightMoveTargetSquareIndices.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightMoveStartSteps.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightMoveTargetSquareIndices[sightIndex] < 0) {
			return getChapter1SquareCenter(chapter1SightCurrentSquareIndices[sightIndex]);
		}

		const auto& settings = chapter1SniperSightSettings;
		const Vec2 startCenter = getChapter1SquareCenter(chapter1SightMoveStartSquareIndices[sightIndex]);
		const Vec2 targetCenter = getChapter1SquareCenter(chapter1SightMoveTargetSquareIndices[sightIndex]);
		const int32 moveAge = localStep - chapter1SightMoveStartSteps[sightIndex];

		if (moveAge <= 0) {
			return startCenter;
		}

		const double t = moveAge / static_cast<double>(Max(settings.sightMoveDurationStep, 1));
		const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
		return startCenter + (targetCenter - startCenter) * rate;
	}

	void createChapter1Line(
		Iwanna::AvoidanceManager& manager,
		const Vec2& start,
		const Vec2& end,
		bool isOuterLine) {

		const auto& settings = chapter1GridSettings;
		const double length = start.distanceFrom(end);
		const int32 count = Max(static_cast<int32>(std::ceil(length / settings.lineSpacing)) + 1, 2);

		for (int32 i = 0; i < count; ++i) {
			const double t = i / static_cast<double>(count - 1);
			manager.createCherry(interpolateLine(start, end, t), Iwanna::Cherry::Settings{
				.textureName = settings.frameTextureName,
				.color = settings.frameColor,
				.canDeleteOutOfScreen = false,
				.canPlayerKill = isOuterLine,
				.depth = isOuterLine ? settings.outerDepth : settings.innerDepth,
				.scale = settings.lineScale,
				.alpha = isOuterLine ? settings.outerLineAlpha : settings.innerLineAlpha,
			});
		}
	}

	void createChapter1SquareFill(
		Iwanna::AvoidanceManager& manager,
		const Vec2& topLeft,
		const Vec2& topRight,
		const Vec2& bottomLeft,
		const Vec2& bottomRight,
		int32 squareIndex) {

		const auto& settings = chapter1GridSettings;
		const double topWidth = topLeft.distanceFrom(topRight);
		const double bottomWidth = bottomLeft.distanceFrom(bottomRight);
		const double leftHeight = topLeft.distanceFrom(bottomLeft);
		const double rightHeight = topRight.distanceFrom(bottomRight);
		const double width = (topWidth + bottomWidth) * 0.5;
		const double height = (leftHeight + rightHeight) * 0.5;
		const int32 columnCount = Max(static_cast<int32>(std::floor(width / settings.fillSpacing)), 1);
		const int32 rowCount = Max(static_cast<int32>(std::floor(height / settings.fillSpacing)), 1);

		auto controller = std::make_shared<Chapter1SquareController>();
		controller->index = squareIndex;
		controller->center = interpolateQuad(topLeft, topRight, bottomLeft, bottomRight, 0.5, 0.5);
		chapter1SquareControllers << controller;

		for (int32 row = 0; row < rowCount; ++row) {
			const double v = (row + 0.5) / static_cast<double>(rowCount);

			for (int32 column = 0; column < columnCount; ++column) {
				const double u = (column + 0.5) / static_cast<double>(columnCount);
				const Vec2 home = interpolateQuad(topLeft, topRight, bottomLeft, bottomRight, u, v);
				manager.createCherry(home, Iwanna::Cherry::Settings{
					.textureName = settings.fillTextureName,
					.color = settings.fillColor,
					.behavior = makeChapter1SquareBehavior(home, controller),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = false,
					.depth = settings.fillDepth,
					.scale = settings.fillScale,
					.alpha = settings.fillAlpha,
					.manualCanPlayerKillControl = true,
				});
			}
		}
	}

	void createChapter1OpeningGrid(Iwanna::AvoidanceManager& manager) {
		chapter1SquareControllers.clear();

		for (int32 row = 0; row < 3; ++row) {
			for (int32 column = 0; column < 3; ++column) {
				createChapter1SquareFill(
					manager,
					chapter1GridPoints[row][column],
					chapter1GridPoints[row][column + 1],
					chapter1GridPoints[row + 1][column],
					chapter1GridPoints[row + 1][column + 1],
					row * 3 + column);
			}
		}

		for (int32 row = 0; row < 4; ++row) {
			for (int32 column = 0; column < 3; ++column) {
				const bool isOuterLine = (row == 0 || row == 3);
				createChapter1Line(
					manager,
					chapter1GridPoints[row][column],
					chapter1GridPoints[row][column + 1],
					isOuterLine);
			}
		}

		for (int32 column = 0; column < 4; ++column) {
			for (int32 row = 0; row < 3; ++row) {
				const bool isOuterLine = (column == 0 || column == 3);
				createChapter1Line(
					manager,
					chapter1GridPoints[row][column],
					chapter1GridPoints[row + 1][column],
					isOuterLine);
			}
		}
	}

	void createChapter1RotatingRings(Iwanna::AvoidanceManager& manager) {
		chapter1RotatingRingControllers.clear();

		const auto& settings = chapter1RotatingRingSettings;
		for (int32 ringIndex = 0; ringIndex < settings.ringCount; ++ringIndex) {
			auto controller = std::make_shared<Chapter1RotatingRingController>();
			controller->center = settings.center;
			const double direction = (ringIndex % 2 == 0) ? 1.0 : -1.0;
			controller->rotationSpeed = direction * (settings.baseRotationSpeed + settings.rotationSpeedStep * ringIndex);
			chapter1RotatingRingControllers << controller;

			const double radius = settings.minRadius + settings.radiusSpacing * ringIndex;
			const int32 cherryCount = getChapter1RotatingRingCherryCount(ringIndex);
			for (int32 i = 0; i < cherryCount; i++) {
				const double angle = 6.283185307179586 * i / Max(cherryCount, 1);
				const Vec2 localOffset{
					Math::Cos(angle) * radius,
					Math::Sin(angle) * radius,
				};

				manager.createCherry(settings.center + localOffset, Iwanna::Cherry::Settings{
					.textureName = settings.textureName,
					.color = settings.color,
					.behavior = makeChapter1RotatingRingBehavior(localOffset, controller),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = settings.canPlayerKill,
					.depth = settings.depth,
					.scale = settings.scale,
					.alpha = settings.color.a,
				});
			}
		}
	}

	std::shared_ptr<Chapter1SquareController> getChapter1SquareController(int32 squareIndex) {
		for (const auto& controller : chapter1SquareControllers) {
			if (controller && controller->index == squareIndex) {
				return controller;
			}
		}

		return nullptr;
	}

	void selectChapter1SniperSightSquares() {
		chapter1SniperSightSquareIndices.clear();
		chapter1SightInitialSquareIndices.clear();
		chapter1SightCurrentSquareIndices.clear();
		chapter1SightMoveStartSquareIndices.clear();
		chapter1SightMoveTargetSquareIndices.clear();
		chapter1SightMoveStartSteps.clear();
		chapter1SightLastActionSteps.clear();
		chapter1SightActionCounts.clear();
		chapter1SightRingTargetCherryIndices.clear();
		chapter1SightRingMoveStartSteps.clear();
		chapter1SightRingMoveStartCenters.clear();
		chapter1SightInitialSquareIndices.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightCurrentSquareIndices.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightMoveStartSquareIndices.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightMoveTargetSquareIndices.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightMoveStartSteps.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightLastActionSteps.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightActionCounts.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightRingTargetCherryIndices.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightRingMoveStartSteps.resize(chapter1SniperSightSettings.sightCount);
		chapter1SightRingMoveStartCenters.resize(chapter1SniperSightSettings.sightCount);
		for (int32 i = 0; i < chapter1SniperSightSettings.sightCount; ++i) {
			chapter1SightInitialSquareIndices[i] = -1;
			chapter1SightCurrentSquareIndices[i] = -1;
			chapter1SightMoveStartSquareIndices[i] = -1;
			chapter1SightMoveTargetSquareIndices[i] = -1;
			chapter1SightMoveStartSteps[i] = -1;
			chapter1SightLastActionSteps[i] = -1;
			chapter1SightActionCounts[i] = 0;
			chapter1SightRingTargetCherryIndices[i] = -1;
			chapter1SightRingMoveStartSteps[i] = -1;
			chapter1SightRingMoveStartCenters[i] = Vec2{ 0,0 };
		}
		chapter1UsedSquareIndices.clear();
		for (const auto& controller : chapter1SquareControllers) {
			if (controller) {
				controller->resetState();
			}
		}

		const auto& settings = chapter1SniperSightSettings;
		Array<int32> candidates = settings.initialCandidateSquareIndices;
		const int32 count = Min(settings.sightCount, static_cast<int32>(candidates.size()));

		for (int32 i = 0; i < count; ++i) {
			const int32 selectedCandidateIndex = Random(i, static_cast<int32>(candidates.size()) - 1);
			std::swap(candidates[i], candidates[selectedCandidateIndex]);
			chapter1SniperSightSquareIndices << candidates[i];
			chapter1SightInitialSquareIndices[i] = candidates[i];
			chapter1SightCurrentSquareIndices[i] = candidates[i];
			chapter1UsedSquareIndices << candidates[i];
		}
	}

	int32 selectChapter1SightMoveTarget(int32 sightIndex) {
		if (sightIndex < 0
			|| chapter1SightActionCounts.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightInitialSquareIndices.size() <= static_cast<size_t>(sightIndex)) {
			return -1;
		}

		if (chapter1SightActionCounts[sightIndex] == 2) {
			return chapter1SightInitialSquareIndices[sightIndex];
		}

		const auto& settings = chapter1SniperSightSettings;
		Array<int32> candidates;

		for (const int32 squareIndex : settings.moveCandidateSquareIndices) {
			if (!chapter1UsedSquareIndices.includes(squareIndex)) {
				candidates << squareIndex;
			}
		}

		if (candidates.isEmpty()) {
			return -1;
		}

		return candidates[Random(static_cast<int32>(candidates.size()) - 1)];
	}

	void activateChapter1PanelBreak(int32 squareIndex, int32 actionStep) {
		const auto& settings = chapter1SniperSightSettings;
		auto controller = getChapter1SquareController(squareIndex);
		if (!controller) {
			return;
		}

		Vec2 direction = getChapter1SquareCenter(squareIndex) - Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		if (direction.length() <= 0.0001) {
			direction = Vec2{ 0.0, -1.0 };
		}

		controller->flyStartStep = actionStep;
		controller->attackStep = actionStep;
		controller->hitColorStartStep = actionStep;
		controller->flyVelocity = direction.normalized() * settings.panelFlySpeed + Vec2{ 0.0, -settings.panelLiftSpeed };
		controller->flyGravity = settings.panelGravity;
		const double rotationDirection = (0.0 <= controller->flyVelocity.x) ? 1.0 : -1.0;
		controller->rotationSpeed = rotationDirection * Random(
			settings.panelMinRotationSpeed,
			settings.panelMaxRotationSpeed);
	}

	void activateChapter1SightAction(Iwanna::AvoidanceManager& manager, int32 sightIndex) {
		const auto& settings = chapter1SniperSightSettings;
		if (sightIndex < 0
			|| chapter1SightCurrentSquareIndices.size() <= static_cast<size_t>(sightIndex)
			|| chapter1SightActionCounts.size() <= static_cast<size_t>(sightIndex)) {
			return;
		}

		const int32 actionStep = getChapter1SightActionStep(
			chapter1SightActionCounts[sightIndex] * settings.sightCount + sightIndex);
		const int32 shotSquareIndex = chapter1SightCurrentSquareIndices[sightIndex];
		const int32 targetSquareIndex = selectChapter1SightMoveTarget(sightIndex);

		activateChapter1PanelBreak(shotSquareIndex, actionStep);

		chapter1SightLastActionSteps[sightIndex] = actionStep;
		if (0 <= targetSquareIndex) {
			chapter1SightMoveStartSquareIndices[sightIndex] = shotSquareIndex;
			chapter1SightMoveTargetSquareIndices[sightIndex] = targetSquareIndex;
			chapter1SightMoveStartSteps[sightIndex] = actionStep;
			chapter1SightCurrentSquareIndices[sightIndex] = targetSquareIndex;

			if (!chapter1UsedSquareIndices.includes(targetSquareIndex)) {
				chapter1UsedSquareIndices << targetSquareIndex;
			}
		}

		++chapter1SightActionCounts[sightIndex];
		manager.requestScreenShake(
			settings.screenShakeAmplitude,
			settings.screenShakeDurationStep,
			settings.screenShakeFrequency);
	}

	void startChapter1SightMoveToInnerRing(int32 localStep) {
		const auto& settings = chapter1SniperSightSettings;
		const int32 targetCount = Min(
			settings.sightCount,
			static_cast<int32>(chapter1SightRingTargetCherryIndices.size()));
		const int32 innerRingCherryCount = getChapter1RotatingRingCherryCount(0);
		const int32 targetSpacing = Max(static_cast<int32>(std::ceil(innerRingCherryCount / static_cast<double>(Max(settings.sightCount, 1)))), 1);
		const int32 redTargetCherryIndex = Random(Min(6, Max(innerRingCherryCount - 1, 0)));

		for (int32 sightIndex = 0; sightIndex < targetCount; ++sightIndex) {
			chapter1SightRingMoveStartCenters[sightIndex] = getChapter1SightCenter(sightIndex, localStep);
			chapter1SightRingMoveStartSteps[sightIndex] = localStep;
			chapter1SightRingTargetCherryIndices[sightIndex] = (redTargetCherryIndex + sightIndex * targetSpacing) % Max(innerRingCherryCount, 1);
		}
	}
}

namespace Iwanna {
	void AvoidanceManager::drawChapter1OpeningFade() const {
		if (activeChapter != 1) {
			return;
		}

		const auto& settings = chapter1OpeningFadeSettings;
		const int32 localStep = step - Global::startStep_Chapter1;
		if (localStep < settings.startStep) {
			Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(settings.color);
			return;
		}

		const int32 fadeStep = localStep - settings.startStep;
		if (settings.durationStep < fadeStep) {
			return;
		}

		const double t = fadeStep / static_cast<double>(Max(settings.durationStep, 1));
		ColorF fadeColor = settings.color;
		fadeColor.a *= 1.0 - applyEasing(EasingMoveType::EaseInOut, t);
		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(fadeColor);
	}

	void AvoidanceManager::drawChapter1SniperSights() const {
		if (activeChapter != 1) {
			return;
		}

		const auto& settings = chapter1SniperSightSettings;
		const int32 localStep = step - Global::startStep_Chapter1;

		for (int32 i = 0; i < static_cast<int32>(chapter1SniperSightSquareIndices.size()); ++i) {
			const Vec2 center = getChapter1SightCenter(i, localStep);
			const ColorF centerColor = settings.centerColors[i % settings.centerColors.size()];
			const double sizeRate = getChapter1SightPulseSizeRate(i, localStep);
			drawChapter1SniperSightAt(center, centerColor, sizeRate, localStep);
		}
	}

	// step :0 - 839
	void AvoidanceManager::chapter1() {
		Timeline timeline(previousStep, step);

		timeline.at(Global::startStep_Chapter1, [&] {
			createChapter1RotatingRings(*this);
			createChapter1OpeningGrid(*this);
			selectChapter1SniperSightSquares();
		});

		const int32 totalActionCount = static_cast<int32>(chapter1SniperSightSettings.moveCandidateSquareIndices.size());
		for (int32 actionIndex = 0; actionIndex < totalActionCount; ++actionIndex) {
			const int32 sightIndex = actionIndex % chapter1SniperSightSettings.sightCount;
			timeline.at(Global::startStep_Chapter1 + getChapter1SightActionStep(actionIndex), [&, sightIndex] {
				activateChapter1SightAction(*this, sightIndex);
			});
		}

		timeline.at(Global::startStep_Chapter1 + 120, [&] {
			startChapter1SightMoveToInnerRing(120);
		});
	}
}
