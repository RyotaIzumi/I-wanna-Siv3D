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
		double outerDepth = Iwanna::DrawDepth::Cherry + 0.3;
		double innerDepth = Iwanna::DrawDepth::Cherry + 0.2;
		double fillDepth = Iwanna::DrawDepth::Cherry + 0.1;
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
		int32 fadeOutStartStep = 318;
		int32 fadeOutStep = 15;
		int32 appearanceStep = 15;
		int32 firstActionStep = 15;
		int32 actionIntervalStep = 12;
		int32 pulseDurationStep = 10;
		double pulseScale = 1.28;
		int32 sightMoveDurationStep = 12;
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
		ColorF outerColor = ColorF{ 0.5, 0.95 };
		ColorF crossColor = ColorF{ 0.5, 0.95 };
		int32 sightCount = 3;
		Array<int32> initialCandidateSquareIndices = { 0, 1, 2, 3, 5, 6, 7, 8 };
		Array<int32> moveCandidateSquareIndices = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
		Array<ColorF> centerColors = {
			ColorF{ 1.0, 0.0, 0.0, 1.0 },
			ColorF{ 0.0, 0.25, 1.0, 1.0 },
			ColorF{ 1.0, 1.0, 0.0 , 1.0 },
		};
	};

	const Chapter1SniperSightSettings chapter1SniperSightSettings{};

	struct Chapter1OpeningFadeSettings {
		int32 startStep = 10;
		int32 durationStep = 10;
		ColorF color = ColorF{ 0.0, 0.0, 0.0, 1.0 };
	};

	const Chapter1OpeningFadeSettings chapter1OpeningFadeSettings{};

	struct Chapter1EndingFadeSettings {
		int32 startStep = 800;
		int32 endStep = 830;
		ColorF color = ColorF{ 0.0, 0.0, 0.0, 1.0 };
	};

	const Chapter1EndingFadeSettings chapter1EndingFadeSettings{};

	struct Chapter1GiantGreenCherrySettings {
		String textureName = U"sprCherryAllWhite";
		ColorF color = ColorF{ 0.0, 0.85, 0.2, 0.52 };
		Vec2 center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		double scale = 31.0;
		double depth = Iwanna::DrawDepth::Block - 1.0;
		bool canPlayerKill = false;
	};

	const Chapter1GiantGreenCherrySettings chapter1GiantGreenCherrySettings{};

	struct Chapter1GreenRadarLineSettings {
		String textureName = U"sprCherryAllWhite";
		ColorF color = ColorF{ 0.0, 0.65, 0.18, 0.9 };
		ColorF centerColor = ColorF{ 1.0, 1.0, 1.0, 1.0 };
		Vec2 center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		double radius = 290.0;
		Array<double> innerRadiusRates = { 0.38, 0.68 };
		double spacing = 4.5;
		double scale = 0.2;
		double centerScale = 0.75;
		int32 crossAppearStep = 318;
		int32 firstCircleAppearStep = 340;
		int32 remainingCircleAppearStep = 362;
		int32 appearDurationStep = 15;
		double depth = Iwanna::DrawDepth::Block - 0.9;
		double centerDepth = Iwanna::DrawDepth::Block - 0.78;
		bool canPlayerKill = false;
	};

	const Chapter1GreenRadarLineSettings chapter1GreenRadarLineSettings{};

	struct Chapter1GreenRadarSweepSettings {
		String textureName = U"sprCherryAllWhite";
		ColorF sweepColor = ColorF{ 1.0, 1.0, 1.0, 1.0 };
		ColorF trailColor = ColorF{ 1.0, 1.0, 1.0, 0.58 };
		Vec2 center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		int32 startStep = 380;
		int32 oneRoundEndStep = 500;
		int32 fourthRoundStartStep = 740;
		int32 fourthRoundSweepEndStep = 790;
		int32 fourthRoundAttackStep = 800;
		int32 trailEndStep = 839;
		int32 trailIntervalStep = 1;
		int32 trailFadeOutStep = 15;
		double fourthRoundAlpha = 0.5;
		double radius = 290.0;
		double sweepSpacing = 4.5;
		double trailSpacing = 4.0;
		double sweepScale = 0.24;
		double trailScale = 0.2;
		double sweepDepth = Iwanna::DrawDepth::Block - 0.82;
		double trailDepth = Iwanna::DrawDepth::Block - 0.84;
		bool canPlayerKill = false;
	};

	const Chapter1GreenRadarSweepSettings chapter1GreenRadarSweepSettings{};

	double getChapter1GreenRadarSweepAlphaRate() {
		return (Global::difficulty == Global::Difficulty::Medium) ? 1.0 : 0.5;
	}

	bool canChapter1GreenRadarSweepKillPlayer() {
		return (Global::difficulty == Global::Difficulty::Medium);
	}

	struct Chapter1GreenRadarSignalSettings {
		String textureName = U"sprCherryAllWhite";
		ColorF color = ColorF{ 1.0, 1.0, 1.0, 1.0 };
		ColorF attackColor = ColorF{ 1.0, 0.0, 0.0, 1.0 };
		int32 count = 150;
		int32 roundCount = 4;
		int32 fadeOutStep = 30;
		int32 vanishStep = 30;
		double radius = 290.0;
		double scale = 0.7;
		double depth = Iwanna::DrawDepth::Block - 0.76;
		bool canPlayerKill = false;
	};

	Chapter1GreenRadarSignalSettings getChapter1GreenRadarSignalSettings() {
		Chapter1GreenRadarSignalSettings settings;
		settings.count = (Global::difficulty == Global::Difficulty::Medium) ? 150 : 100;
		return settings;
	}

	struct Chapter1SightBarrageSettings {
		String textureName = U"sprCherryAllWhite";
		int32 cherryCount = 36;
		int32 ringCount = 3;
		double targetRadius = 72.0;
		double radiusInterval = 26.0;
		int32 expansionStep = 10;
		int32 fadeOutStep = 15;
		double scale = 0.58;
		double depth = Iwanna::DrawDepth::Cherry + 6.0;
	};

	const Chapter1SightBarrageSettings chapter1SightBarrageSettings{};

	struct Chapter1BlackRingBarrageSettings {
		String textureName = U"sprCherryWhite";
		ColorF color = ColorF{ 0.2, 0.2, 0.2, 1.0 };
		Vec2 center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		int32 startStep = 250;
		int32 arrivalStep = 320;
		int32 rotationStartStep = 380;
		int32 ringCount = 9;
		int32 baseCherryCount = 44;
		double innerRadius = 320.0;
		double radiusInterval = 32.0;
		double startRadius = 680.0;
		double rotationSpeedStepRate = 3;
		double scale = 4.8;
		double depth = Iwanna::DrawDepth::Player + 11.0;
		bool canPlayerKill = true;
	};

	const Chapter1BlackRingBarrageSettings chapter1BlackRingBarrageSettings{};

	struct Chapter1RedSightRadialBarrageSettings {
		String textureName = U"sprCherryWhite";
		int32 startStep = 136;
		Array<int32> sightStartSteps = { 136, 200, 260 };
		int32 repeatIntervalStep = 3;
		//int32 directionCount = 4;
		double baseAngle = 90.0;
		double angleShiftPerShot = 53.0;
		double speed = 7.5;
		double scale = 1.0;
		double alpha = 1.0;
		double passiveScale = 0.5;
		double activeScale = 1.0;
		double passiveAlpha = 0.65;
		double activeAlpha = 1.0;
		int32 transitionStep = 6;
		double deleteMargin = 160.0;
		int32 slowDownStartStep = 260;
		int32 slowDownEndStep = 318;
		int32 fadeOutStep = 50;
		double bounceMinSpeed = 4.0;
		double bounceMaxSpeed = 9.0;
		double bounceGravity = 0.32;
		double depth = Iwanna::DrawDepth::Cherry + 6.5;
	};

	const Chapter1RedSightRadialBarrageSettings chapter1RedSightRadialBarrageSettings{};

	int32 getChapter1RedSightRadialBarrageDirectionCount() {
		return (Global::difficulty == Global::Difficulty::Medium) ? 5 : 4;
	}

	struct Chapter1RedSightRadialBarrageController {
		double rate = 1.0;
		double startRate = 1.0;
		double targetRate = 1.0;
		double scale = 1.0;
		double alpha = 1.0;
		int32 timer = 0;
		int32 transitionStep = 1;

		void request(bool isAttackState, int32 newTransitionStep) {
			startRate = rate;
			targetRate = isAttackState ? 1.0 : 0.0;
			timer = 0;
			transitionStep = Max(newTransitionStep, 1);
		}

		void update(const Chapter1RedSightRadialBarrageSettings& settings) {
			if (rate != targetRate) {
				const double t = timer / static_cast<double>(transitionStep);
				const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);
				rate = startRate + (targetRate - startRate) * eased;

				if (transitionStep <= timer) {
					rate = targetRate;
				}

				++timer;
			}

			scale = settings.passiveScale + (settings.activeScale - settings.passiveScale) * rate;
			alpha = settings.passiveAlpha + (settings.activeAlpha - settings.passiveAlpha) * rate;
		}

		bool canPlayerKill() const {
			return 0.999 <= rate;
		}
	};

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
		int32 fadeOutStep = 15;
		double depth = Iwanna::DrawDepth::Cherry + 0.5;
		bool canPlayerKill = true;
	};

	const Chapter1RotatingRingSettings chapter1RotatingRingSettings{};

	struct Chapter1RotatingRingController {
		Vec2 center = Vec2{ 0,0 };
		double rotationSpeed = 0.0;
		int32 stopStep = -1;
	};

	struct Chapter1FrameFallSettings {
		int32 startStep = 120;
		Vec2 velocity = Vec2{ 0.0, 1.3 };
		double gravity = 0.22;
		double rotationSpeed = 0.018;
		double bottomDeleteMargin = 48.0;
	};

	const Chapter1FrameFallSettings chapter1FrameFallSettings{};

	struct Chapter1FrameController {
		Vec2 center = Vec2{ 0,0 };
		Vec2 velocity = Vec2{ 0,0 };
		double gravity = 0.0;
		double rotationSpeed = 0.0;
		int32 fallStartStep = -1;
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
	std::shared_ptr<Chapter1FrameController> chapter1FrameController;
	Array<std::shared_ptr<Chapter1RedSightRadialBarrageController>> chapter1SightRadialBarrageControllers;
	int32 chapter1MotionStopStep = -1;
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
	int32 chapter1SightRadialBarrageDeleteStartStep = -1;

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
			const int32 activeAge = (controller && 0 <= controller->stopStep)
				? Min(age, controller->stopStep)
				: age;
			const double angle = controller->rotationSpeed * activeAge;
			const double c = Math::Cos(angle);
			const double s = Math::Sin(angle);
			const Vec2 rotated{
				localOffset.x * c - localOffset.y * s,
				localOffset.x * s + localOffset.y * c,
			};

			self.pos = controller->center + rotated;

			const auto& settings = chapter1RotatingRingSettings;
			self.canPlayerKill = settings.canPlayerKill;
			self.alpha = settings.color.a;
			if (controller && 0 <= controller->stopStep && controller->stopStep <= age) {
				const int32 fadeAge = age - controller->stopStep;
				const double t = fadeAge / static_cast<double>(Max(settings.fadeOutStep, 1));
				self.canPlayerKill = false;
				self.alpha = settings.color.a * (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t));

				if (settings.fadeOutStep <= fadeAge) {
					self.isDelete = true;
				}
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter1SightBarrageBehavior(
		const Vec2& center,
		const Vec2& targetOffset) {

		return [center, targetOffset](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter1SightBarrageSettings;
			if (age <= settings.expansionStep) {
				const double t = age / static_cast<double>(Max(settings.expansionStep, 1));
				const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
				self.pos = center + targetOffset * rate;
				self.canPlayerKill = true;
				self.alpha = 1.0;
				return;
			}

			const int32 fadeAge = age - settings.expansionStep;
			const double fadeT = fadeAge / static_cast<double>(Max(settings.fadeOutStep, 1));
			self.pos = center + targetOffset;
			self.canPlayerKill = false;
			self.alpha = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, fadeT);

			if (settings.fadeOutStep <= fadeAge) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter1BlackRingBarrageBehavior(
		const Vec2& startOffset,
		const Vec2& targetOffset,
		int32 ringIndex) {

		return [startOffset, targetOffset, ringIndex](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter1BlackRingBarrageSettings;
			const auto& sweepSettings = chapter1GreenRadarSweepSettings;
			const int32 moveStep = Max(settings.arrivalStep - settings.startStep, 1);
			const double t = age / static_cast<double>(moveStep);
			const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			Vec2 currentTargetOffset = targetOffset;

			const int32 localStep = settings.startStep + age;
			if (settings.rotationStartStep <= localStep) {
				const int32 sonarRoundStep = Max(sweepSettings.oneRoundEndStep - sweepSettings.startStep, 1);
				const double baseRotationSpeed = 2.0 * Math::Pi / sonarRoundStep;
				const double rotationSpeed = baseRotationSpeed + Math::ToRadians(Abs(settings.rotationSpeedStepRate)) * ringIndex;
				const double rotation = rotationSpeed * (localStep - settings.rotationStartStep);
				const double c = Math::Cos(rotation);
				const double s = Math::Sin(rotation);
				currentTargetOffset = Vec2{
					targetOffset.x * c - targetOffset.y * s,
					targetOffset.x * s + targetOffset.y * c,
				};
			}

			self.pos = settings.center + startOffset + (currentTargetOffset - startOffset) * rate;
		};
	}

	double getChapter1SightRadialBarrageSpeedRate(int32 localStep) {
		const auto& settings = chapter1RedSightRadialBarrageSettings;
		if (localStep < settings.slowDownStartStep) {
			return 1.0;
		}

		if (settings.slowDownEndStep <= localStep) {
			return 0.0;
		}

		const int32 slowDownStep = Max(settings.slowDownEndStep - settings.slowDownStartStep, 1);
		const double t = (localStep - settings.slowDownStartStep) / static_cast<double>(slowDownStep);
		return 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);
	}

	double getChapter1SightRadialBarrageMoveAge(int32 spawnLocalStep, int32 currentLocalStep) {
		const int32 toStep = Max(currentLocalStep, spawnLocalStep);
		double moveAge = 0.0;

		for (int32 localStep = spawnLocalStep; localStep < toStep; ++localStep) {
			moveAge += getChapter1SightRadialBarrageSpeedRate(localStep);
		}

		return moveAge;
	}

	Iwanna::Cherry::Behavior makeChapter1RedSightRadialBarrageBehavior(
		const Vec2& start,
		const Vec2& velocity,
		int32 spawnLocalStep,
		const std::shared_ptr<Chapter1RedSightRadialBarrageController>& controller,
		bool usesSlowDown) {

		return [
			start,
			velocity,
			spawnLocalStep,
			controller,
			usesSlowDown,
			isDeleteEffectStarted = false,
			deleteEffectStartPos = Vec2{ 0, 0 },
			bounceVelocity = Vec2{ 0, 0 },
			deleteEffectStartAlpha = 1.0
		](Iwanna::Cherry& self, int32 age) mutable {
			const auto& settings = chapter1RedSightRadialBarrageSettings;
			const int32 currentLocalStep = spawnLocalStep + age;
			const int32 activeLocalStep = (0 <= chapter1MotionStopStep)
				? Min(currentLocalStep, chapter1MotionStopStep)
				: currentLocalStep;
			const double moveAge = usesSlowDown
				? getChapter1SightRadialBarrageMoveAge(spawnLocalStep, activeLocalStep)
				: Max(activeLocalStep - spawnLocalStep, 0);
			self.pos = start + velocity * moveAge;

			if (controller) {
				self.canPlayerKill = controller->canPlayerKill();
				self.setScale(controller->scale);
				self.alpha = controller->alpha;
			}

			if (0 <= chapter1SightRadialBarrageDeleteStartStep
				&& chapter1SightRadialBarrageDeleteStartStep <= currentLocalStep) {

				if (!isDeleteEffectStarted) {
					isDeleteEffectStarted = true;
					deleteEffectStartPos = self.pos;
					deleteEffectStartAlpha = self.alpha;
					const double angle = Random(0.0, 2.0 * Math::Pi);
					const double speed = Random(settings.bounceMinSpeed, settings.bounceMaxSpeed);
					bounceVelocity = Vec2{ Math::Cos(angle), Math::Sin(angle) } * speed;
				}

				const int32 deleteAge = currentLocalStep - chapter1SightRadialBarrageDeleteStartStep;
				const double fadeT = deleteAge / static_cast<double>(Max(settings.fadeOutStep, 1));
				self.pos = deleteEffectStartPos
					+ bounceVelocity * deleteAge
					+ Vec2{ 0.0, 0.5 * settings.bounceGravity * deleteAge * deleteAge };
				self.canPlayerKill = false;
				self.alpha = deleteEffectStartAlpha * (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, fadeT));

				if (settings.fadeOutStep <= deleteAge) {
					self.isDelete = true;
				}
				return;
			}

			const double margin = Max(settings.deleteMargin, 0.0);
			if (self.pos.x < -margin
				|| Global::windowWidth + margin < self.pos.x
				|| self.pos.y < -margin
				|| Global::windowHeight + margin < self.pos.y) {
				self.isDelete = true;
			}
		};
	}

	double getChapter1GreenRadarSweepAngle(int32 localStep) {
		const auto& settings = chapter1GreenRadarSweepSettings;
		if (settings.fourthRoundStartStep <= localStep) {
			const int32 rotationStep = Max(settings.fourthRoundSweepEndStep - settings.fourthRoundStartStep, 1);
			const double t = (localStep - settings.fourthRoundStartStep) / static_cast<double>(rotationStep);
			const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			return -(Math::Pi / 2.0) + 2.0 * Math::Pi * rate;
		}

		const int32 rotationStep = Max(settings.oneRoundEndStep - settings.startStep, 1);
		const double t = (localStep - settings.startStep) / static_cast<double>(rotationStep);
		return -(Math::Pi / 2.0) + 2.0 * Math::Pi * t;
	}

	double normalizeChapter1AngleProgress(double angle) {
		const double fullTurn = 2.0 * Math::Pi;
		while (angle < 0.0) {
			angle += fullTurn;
		}
		while (fullTurn <= angle) {
			angle -= fullTurn;
		}
		return angle / fullTurn;
	}

	Iwanna::Cherry::Behavior makeChapter1GreenRadarSweepBehavior(double distanceFromCenter) {
		return [distanceFromCenter](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter1GreenRadarSweepSettings;
			const int32 localStep = settings.startStep + age;
			const double angle = getChapter1GreenRadarSweepAngle(localStep);
			const Vec2 direction{ Math::Cos(angle), Math::Sin(angle) };
			const double alphaRate = getChapter1GreenRadarSweepAlphaRate();

			self.pos = settings.center + direction * distanceFromCenter;
			self.canPlayerKill = canChapter1GreenRadarSweepKillPlayer()
				&& (localStep < settings.fourthRoundStartStep);
			self.alpha = (settings.fourthRoundStartStep <= localStep)
				? settings.fourthRoundAlpha * alphaRate
				: settings.sweepColor.a * alphaRate;
		};
	}

	Iwanna::Cherry::Behavior makeChapter1GreenRadarSweepTrailBehavior(double initialAlpha) {
		return [initialAlpha](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter1GreenRadarSweepSettings;
			const double alphaRate = getChapter1GreenRadarSweepAlphaRate();
			const double t = age / static_cast<double>(Max(settings.trailFadeOutStep, 1));
			self.canPlayerKill = false;
			self.alpha = initialAlpha * alphaRate * (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t));

			if (settings.trailFadeOutStep <= age) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter1GreenRadarSignalBehavior(
		int32 roundStartStep,
		int32 roundEndStep,
		int32 revealStep,
		double revealAlpha) {

		return [roundStartStep, roundEndStep, revealStep, revealAlpha](Iwanna::Cherry& self, int32 age) {
			const auto signalSettings = getChapter1GreenRadarSignalSettings();
			const int32 localStep = roundStartStep + age;

			self.setScale(signalSettings.scale);
			self.canPlayerKill = false;
			self.setColor(signalSettings.color);

			if (localStep < revealStep) {
				self.alpha = 0.0;
				return;
			}

			if (localStep < roundEndStep) {
				const int32 fadeAge = localStep - revealStep;
				const double fadeT = fadeAge / static_cast<double>(Max(signalSettings.fadeOutStep, 1));
				self.alpha = revealAlpha
					* (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, fadeT));
				return;
			}

			if (localStep == roundEndStep) {
				self.setColor(signalSettings.attackColor);
				self.alpha = signalSettings.attackColor.a;
				self.canPlayerKill = true;
				return;
			}

			self.setColor(signalSettings.attackColor);
			const int32 vanishAge = localStep - roundEndStep;
			const double vanishT = vanishAge / static_cast<double>(Max(signalSettings.vanishStep, 1));
			const double rate = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, vanishT);
			self.alpha = signalSettings.attackColor.a * rate;
			self.setScale(signalSettings.scale * rate);

			if (signalSettings.vanishStep <= vanishAge) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter1GreenRadarLineAppearanceBehavior(
		int32 appearStep,
		double targetScale,
		double targetAlpha) {

		return [appearStep, targetScale, targetAlpha](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter1GreenRadarLineSettings;
			self.canPlayerKill = false;

			if (age < appearStep) {
				self.setScale(0.0);
				self.alpha = 0.0;
				return;
			}

			const int32 appearAge = age - appearStep;
			const double t = appearAge / static_cast<double>(Max(settings.appearDurationStep, 1));
			const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			self.setScale(targetScale * rate);
			self.alpha = targetAlpha * rate;
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

	Iwanna::Cherry::Behavior makeChapter1FrameBehavior(
		const Vec2& home,
		bool isOuterLine,
		const std::shared_ptr<Chapter1FrameController>& controller) {

		return [home, isOuterLine, controller](Iwanna::Cherry& self, int32) {
			if (!controller || controller->fallStartStep < 0 || self.getAge() < controller->fallStartStep) {
				self.pos = home;
				self.textureAngle = 0.0;
				self.canPlayerKill = isOuterLine;
				return;
			}

			const auto& gridSettings = chapter1GridSettings;
			const double fallAge = static_cast<double>(self.getAge() - controller->fallStartStep);
			const double angle = controller->rotationSpeed * fallAge;
			const double c = Math::Cos(angle);
			const double s = Math::Sin(angle);
			const Vec2 localPos = home - controller->center;
			const Vec2 rotated{
				localPos.x * c - localPos.y * s,
				localPos.x * s + localPos.y * c,
			};
			const Vec2 offset = controller->velocity * fallAge
				+ Vec2{ 0.0, 0.5 * controller->gravity * fallAge * fallAge };

			self.pos = controller->center + rotated + offset;
			self.textureAngle = angle;
			self.canPlayerKill = false;

			if (isOuterLine) {
				ColorF color = gridSettings.frameColor;
				color.a = gridSettings.innerLineAlpha;
				self.setColor(color);
			}

			if (Global::windowHeight + chapter1FrameFallSettings.bottomDeleteMargin < self.pos.y) {
				self.isDelete = true;
			}
		};
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
		int32 localStep,
		double alphaRate) {

		const auto& settings = chapter1SniperSightSettings;
		const double appearanceRate = Iwanna::applyEasing(
			Iwanna::EasingMoveType::EaseOut,
			localStep / static_cast<double>(Max(settings.appearanceStep, 1)));
		const double visibleRadius = settings.radius * appearanceRate * sizeRate;
		const double lineRadius = (settings.radius + settings.crossExtend) * appearanceRate * sizeRate;
		const auto texture = TextureAsset(settings.textureName);
		const int32 activeLocalStep = (0 <= chapter1MotionStopStep)
			? Min(localStep, chapter1MotionStopStep)
			: localStep;
		const int32 rotationStep = Max(activeLocalStep - settings.rotationStartStep, 0);
		const double rotation = rotationStep * settings.rotationSpeed;
		const auto rotate = [rotation](const Vec2& v) {
			const double c = Math::Cos(rotation);
			const double s = Math::Sin(rotation);
			return Vec2{
				v.x * c - v.y * s,
				v.x * s + v.y * c,
			};
		};

		ColorF outerColor = settings.outerColor;
		ColorF crossColor = settings.crossColor;
		ColorF visibleCenterColor = centerColor;
		outerColor.a *= alphaRate;
		crossColor.a *= alphaRate;
		visibleCenterColor.a *= alphaRate;

		for (int32 i = 0; i < settings.circleCherryCount; ++i) {
			const double angle = 6.283185307179586 * i / Max(settings.circleCherryCount, 1) + rotation;
			const Vec2 pos = center + Vec2{ Math::Cos(angle), Math::Sin(angle) } * visibleRadius;
			texture.scaled(settings.outerScale).drawAt(pos, outerColor);
		}

		for (double offset = -(settings.radius + settings.crossExtend); offset <= settings.radius + settings.crossExtend; offset += settings.crossSpacing) {
			if (Abs(offset) <= settings.crossSpacing * 0.5) {
				continue;
			}

			const double visibleOffset = offset * appearanceRate * sizeRate;
			if (Abs(visibleOffset) <= lineRadius) {
				texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ visibleOffset, 0.0 }), crossColor);
				texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ 0.0, visibleOffset }), crossColor);
			}
		}

		texture.scaled(settings.centerScale * sizeRate).drawAt(center, visibleCenterColor);
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
		const int32 activeLocalStep = (0 <= chapter1MotionStopStep)
			? Min(localStep, chapter1MotionStopStep)
			: localStep;
		const int32 cherryCount = getChapter1RotatingRingCherryCount(ringIndex);
		const double radius = settings.minRadius + settings.radiusSpacing * ringIndex;
		const double direction = (ringIndex % 2 == 0) ? 1.0 : -1.0;
		const double rotationSpeed = direction * (settings.baseRotationSpeed + settings.rotationSpeedStep * ringIndex);
		const double angle = 6.283185307179586 * (cherryIndex % Max(cherryCount, 1)) / Max(cherryCount, 1)
			+ rotationSpeed * activeLocalStep;

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
			const Vec2 home = interpolateLine(start, end, t);
			manager.createCherry(home, Iwanna::Cherry::Settings{
				.textureName = settings.frameTextureName,
				.color = settings.frameColor,
				.behavior = makeChapter1FrameBehavior(home, isOuterLine, chapter1FrameController),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = isOuterLine,
				.depth = isOuterLine ? settings.outerDepth : settings.innerDepth,
				.scale = settings.lineScale,
				.alpha = isOuterLine ? settings.outerLineAlpha : settings.innerLineAlpha,
				.manualCanPlayerKillControl = true,
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
		chapter1FrameController = std::make_shared<Chapter1FrameController>();
		chapter1FrameController->center = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		chapter1FrameController->velocity = chapter1FrameFallSettings.velocity;
		chapter1FrameController->gravity = chapter1FrameFallSettings.gravity;
		chapter1FrameController->rotationSpeed = chapter1FrameFallSettings.rotationSpeed;

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

	void startChapter1FrameFall() {
		if (chapter1FrameController) {
			chapter1FrameController->fallStartStep = chapter1FrameFallSettings.startStep;
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
					.manualCanPlayerKillControl = true,
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

	void createChapter1SightBarrageAt(
		Iwanna::AvoidanceManager& manager,
		const Vec2& center,
		const ColorF& color) {

		const auto& settings = chapter1SightBarrageSettings;
		for (int32 ringIndex = 0; ringIndex < settings.ringCount; ++ringIndex) {
			const double radius = Max(settings.targetRadius - settings.radiusInterval * ringIndex, 0.0);
			for (int32 i = 0; i < settings.cherryCount; ++i) {
				const double angle = 6.283185307179586 * i / Max(settings.cherryCount, 1);
				const Vec2 targetOffset{
					Math::Cos(angle) * radius,
					Math::Sin(angle) * radius,
				};

				manager.createCherry(center, Iwanna::Cherry::Settings{
					.textureName = settings.textureName,
					.color = color,
					.behavior = makeChapter1SightBarrageBehavior(center, targetOffset),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = true,
					.depth = settings.depth,
					.scale = settings.scale,
					.alpha = color.a,
					.canPlayerKillBeforeFullAlpha = true,
					.manualCanPlayerKillControl = true,
				});
			}
		}
	}

	void createChapter1SightBarrages(Iwanna::AvoidanceManager& manager, int32 localStep) {
		const auto& settings = chapter1SniperSightSettings;
		const int32 count = Min(
			settings.sightCount,
			static_cast<int32>(chapter1SniperSightSquareIndices.size()));

		for (int32 sightIndex = 0; sightIndex < count; ++sightIndex) {
			createChapter1SightBarrageAt(
				manager,
				getChapter1SightCenter(sightIndex, localStep),
				settings.centerColors[sightIndex % settings.centerColors.size()]);
		}
	}

	void createChapter1GiantGreenCherry(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter1GiantGreenCherrySettings;
		manager.createCherry(settings.center, Iwanna::Cherry::Settings{
			.textureName = settings.textureName,
			.color = settings.color,
			.canDeleteOutOfScreen = false,
			.canPlayerKill = settings.canPlayerKill,
			.depth = settings.depth,
			.scale = settings.scale,
			.alpha = settings.color.a,
		});
	}

	void createChapter1GreenRadarCircle(
		Iwanna::AvoidanceManager& manager,
		double radius,
		int32 appearStep) {

		const auto& settings = chapter1GreenRadarLineSettings;
		const double circumference = 2.0 * Math::Pi * radius;
		const int32 count = Max(static_cast<int32>(std::ceil(circumference / settings.spacing)), 12);

		for (int32 i = 0; i < count; ++i) {
			const double angle = 2.0 * Math::Pi * i / count;
			const Vec2 pos = settings.center + Vec2{ Math::Cos(angle), Math::Sin(angle) } * radius;
			manager.createCherry(pos, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter1GreenRadarLineAppearanceBehavior(
					appearStep,
					settings.scale,
					settings.color.a),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = settings.canPlayerKill,
				.depth = settings.depth,
				.scale = 0.0,
				.alpha = 0.0,
				.manualCanPlayerKillControl = true,
			});
		}
	}

	void createChapter1GreenRadarLine(
		Iwanna::AvoidanceManager& manager,
		const Vec2& start,
		const Vec2& end,
		int32 appearStep) {

		const auto& settings = chapter1GreenRadarLineSettings;
		const double length = start.distanceFrom(end);
		const int32 count = Max(static_cast<int32>(std::ceil(length / settings.spacing)) + 1, 2);

		for (int32 i = 0; i < count; ++i) {
			const double t = i / static_cast<double>(count - 1);
			const Vec2 pos = start + (end - start) * t;
			manager.createCherry(pos, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter1GreenRadarLineAppearanceBehavior(
					appearStep,
					settings.scale,
					settings.color.a),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = settings.canPlayerKill,
				.depth = settings.depth,
				.scale = 0.0,
				.alpha = 0.0,
				.manualCanPlayerKillControl = true,
			});
		}
	}

	void createChapter1GreenRadarLines(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter1GreenRadarLineSettings;
		for (int32 i = 0; i < static_cast<int32>(settings.innerRadiusRates.size()); ++i) {
			const int32 appearStep = (i == 0)
				? settings.firstCircleAppearStep
				: settings.remainingCircleAppearStep;
			createChapter1GreenRadarCircle(manager, settings.radius * settings.innerRadiusRates[i], appearStep);
		}
		createChapter1GreenRadarCircle(manager, settings.radius, settings.remainingCircleAppearStep);

		createChapter1GreenRadarLine(
			manager,
			settings.center + Vec2{ -settings.radius, 0.0 },
			settings.center + Vec2{ settings.radius, 0.0 },
			settings.crossAppearStep);
		createChapter1GreenRadarLine(
			manager,
			settings.center + Vec2{ 0.0, -settings.radius },
			settings.center + Vec2{ 0.0, settings.radius },
			settings.crossAppearStep);

		manager.createCherry(settings.center, Iwanna::Cherry::Settings{
			.textureName = settings.textureName,
			.color = settings.centerColor,
			.behavior = makeChapter1GreenRadarLineAppearanceBehavior(
				settings.crossAppearStep,
				settings.centerScale,
				settings.centerColor.a),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = settings.canPlayerKill,
			.depth = settings.centerDepth,
			.scale = 0.0,
			.alpha = 0.0,
			.manualCanPlayerKillControl = true,
		});
	}

	void createChapter1GreenRadarSweepLine(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter1GreenRadarSweepSettings;
		const int32 count = Max(static_cast<int32>(std::ceil(settings.radius / settings.sweepSpacing)) + 1, 2);

		for (int32 i = 0; i < count; ++i) {
			const double distance = settings.radius * i / static_cast<double>(count - 1);
			manager.createCherry(settings.center + Vec2{ 0.0, -distance }, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.sweepColor,
				.behavior = makeChapter1GreenRadarSweepBehavior(distance),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = settings.canPlayerKill,
				.depth = settings.sweepDepth,
				.scale = settings.sweepScale,
				.alpha = settings.sweepColor.a,
				.manualCanPlayerKillControl = true,
			});
		}
	}

	void createChapter1GreenRadarSweepTrail(Iwanna::AvoidanceManager& manager, int32 localStep) {
		const auto& settings = chapter1GreenRadarSweepSettings;
		const double angle = getChapter1GreenRadarSweepAngle(localStep);
		const Vec2 direction{ Math::Cos(angle), Math::Sin(angle) };
		const int32 count = Max(static_cast<int32>(std::ceil(settings.radius / settings.trailSpacing)) + 1, 2);
		const double initialAlpha = (settings.fourthRoundStartStep <= localStep)
			? settings.fourthRoundAlpha
			: settings.trailColor.a;

		for (int32 i = 0; i < count; ++i) {
			const double distance = settings.radius * i / static_cast<double>(count - 1);
			const Vec2 pos = settings.center + direction * distance;
			manager.createCherry(pos, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.trailColor,
				.behavior = makeChapter1GreenRadarSweepTrailBehavior(initialAlpha),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = settings.canPlayerKill,
				.depth = settings.trailDepth,
				.scale = settings.trailScale,
				.alpha = initialAlpha,
				.manualCanPlayerKillControl = true,
			});
		}
	}

	void createChapter1GreenRadarSignals(
		Iwanna::AvoidanceManager& manager,
		int32 roundStartStep,
		int32 sweepEndStep,
		int32 attackStep,
		bool usesEaseOutSweep) {

		const auto signalSettings = getChapter1GreenRadarSignalSettings();
		const auto& sweepSettings = chapter1GreenRadarSweepSettings;
		const int32 rotationStep = Max(sweepEndStep - roundStartStep, 1);
		const double sweepStartAngle = -(Math::Pi / 2.0);
		const double revealAlpha = usesEaseOutSweep
			? sweepSettings.fourthRoundAlpha
			: signalSettings.color.a;

		for (int32 i = 0; i < signalSettings.count; ++i) {
			const double angle = Random(0.0, 2.0 * Math::Pi);
			const double radius = signalSettings.radius * Math::Sqrt(Random(0.0, 1.0));
			const Vec2 offset{ Math::Cos(angle) * radius, Math::Sin(angle) * radius };
			const Vec2 pos = sweepSettings.center + offset;
			const double signalAngle = std::atan2(offset.y, offset.x);
			const double progress = normalizeChapter1AngleProgress(signalAngle - sweepStartAngle);
			const double revealRate = usesEaseOutSweep
				? 1.0 - Math::Sqrt(1.0 - progress)
				: progress;
			const int32 revealStep = roundStartStep
				+ static_cast<int32>(std::round(revealRate * rotationStep));

			manager.createCherry(pos, Iwanna::Cherry::Settings{
				.textureName = signalSettings.textureName,
				.color = signalSettings.color,
				.behavior = makeChapter1GreenRadarSignalBehavior(
					roundStartStep,
					attackStep,
					revealStep,
					revealAlpha),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = signalSettings.canPlayerKill,
				.depth = signalSettings.depth,
				.scale = signalSettings.scale,
				.alpha = 0.0,
				.manualCanPlayerKillControl = true,
			});
		}
	}

	void createChapter1BlackRingBarrage(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter1BlackRingBarrageSettings;
		for (int32 ringIndex = 0; ringIndex < settings.ringCount; ++ringIndex) {
			const double radius = settings.innerRadius + settings.radiusInterval * ringIndex;
			const int32 cherryCount = Max(
				static_cast<int32>(std::round(settings.baseCherryCount * radius / Max(settings.innerRadius, 1.0))),
				settings.baseCherryCount);

			for (int32 i = 0; i < cherryCount; ++i) {
				const double angle = 6.283185307179586 * i / Max(cherryCount, 1);
				const Vec2 targetOffset{
					Math::Cos(angle) * radius,
					Math::Sin(angle) * radius,
				};
				const Vec2 startOffset{
					Math::Cos(angle) * settings.startRadius,
					Math::Sin(angle) * settings.startRadius,
				};

				manager.createCherry(settings.center + startOffset, Iwanna::Cherry::Settings{
					.textureName = settings.textureName,
					.color = settings.color,
					.behavior = makeChapter1BlackRingBarrageBehavior(startOffset, targetOffset, ringIndex),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = settings.canPlayerKill,
					.depth = settings.depth,
					.scale = settings.scale,
					.alpha = settings.color.a,
				});
			}
		}
	}

	void initializeChapter1RedSightRadialBarrageController() {
		const auto& settings = chapter1RedSightRadialBarrageSettings;
		const int32 count = Max(chapter1SniperSightSettings.sightCount, 1);
		chapter1SightRadialBarrageControllers.clear();
		chapter1SightRadialBarrageControllers.resize(count);

		for (auto& controller : chapter1SightRadialBarrageControllers) {
			controller = std::make_shared<Chapter1RedSightRadialBarrageController>();
			controller->rate = 1.0;
			controller->targetRate = 1.0;
			controller->transitionStep = Max(settings.transitionStep, 1);
			controller->update(settings);
		}
	}

	std::shared_ptr<Chapter1RedSightRadialBarrageController> getChapter1SightRadialBarrageController(int32 sightIndex) {
		if (chapter1SightRadialBarrageControllers.isEmpty()
			|| sightIndex < 0
			|| chapter1SightRadialBarrageControllers.size() <= static_cast<size_t>(sightIndex)) {
			initializeChapter1RedSightRadialBarrageController();
		}

		if (sightIndex < 0
			|| chapter1SightRadialBarrageControllers.size() <= static_cast<size_t>(sightIndex)) {
			return nullptr;
		}

		return chapter1SightRadialBarrageControllers[sightIndex];
	}

	void setChapter1SightRadialBarrageState(int32 sightIndex, bool isAttackState) {
		auto controller = getChapter1SightRadialBarrageController(sightIndex);
		if (!controller) {
			return;
		}

		controller->request(
			isAttackState,
			chapter1RedSightRadialBarrageSettings.transitionStep);
	}

	void createChapter1SightRadialBarrage(Iwanna::AvoidanceManager& manager, int32 sightIndex, int32 localStep) {
		const auto& settings = chapter1RedSightRadialBarrageSettings;
		const auto& sightSettings = chapter1SniperSightSettings;
		if (0 <= chapter1MotionStopStep && chapter1MotionStopStep <= localStep) {
			return;
		}

		if (sightIndex < 0
			|| chapter1SniperSightSquareIndices.size() <= static_cast<size_t>(sightIndex)
			|| sightSettings.centerColors.isEmpty()) {
			return;
		}

		auto controller = getChapter1SightRadialBarrageController(sightIndex);
		const int32 sightStartStep = settings.sightStartSteps[
			sightIndex % settings.sightStartSteps.size()];
		const int32 repeatIndex = Max((localStep - sightStartStep) / Max(settings.repeatIntervalStep, 1), 0);
		const Vec2 center = getChapter1SightCenter(sightIndex, localStep);
		const ColorF color = sightSettings.centerColors[sightIndex % sightSettings.centerColors.size()];
		const int32 directionCount = getChapter1RedSightRadialBarrageDirectionCount();
		const double angleInterval = 360.0 / Max(directionCount, 1);
		const double baseAngle = settings.baseAngle + settings.angleShiftPerShot * repeatIndex;
		const bool usesSlowDown = (sightIndex == 0);

		for (int32 i = 0; i < directionCount; ++i) {
			const double direction = baseAngle + angleInterval * i;
			const double rad = Math::ToRadians(direction);
			const Vec2 velocity{
				settings.speed * Math::Cos(rad),
				-settings.speed * Math::Sin(rad),
			};

			manager.createCherry(center, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = color,
				.behavior = makeChapter1RedSightRadialBarrageBehavior(
					center,
					velocity,
					localStep,
					controller,
					usesSlowDown),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = controller
					? controller->canPlayerKill()
					: true,
				.depth = settings.depth,
				.scale = controller
					? controller->scale
					: settings.activeScale,
				.alpha = controller
					? controller->alpha
					: settings.activeAlpha,
				.canPlayerKillBeforeFullAlpha = true,
				.manualCanPlayerKillControl = true,
			});
		}
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

	void stopChapter1Motion(int32 localStep) {
		chapter1MotionStopStep = localStep;
		for (const auto& controller : chapter1RotatingRingControllers) {
			if (controller) {
				controller->stopStep = localStep;
			}
		}
	}
}

namespace Iwanna {
	void AvoidanceManager::requestChapter1RedSightRadialBarrageState(bool isAttackState) {
		setChapter1SightRadialBarrageState(0, isAttackState);
	}

	void AvoidanceManager::requestChapter1BlueSightRadialBarrageState(bool isAttackState) {
		setChapter1SightRadialBarrageState(1, isAttackState);
	}

	void AvoidanceManager::requestChapter1GreenSightRadialBarrageState(bool isAttackState) {
		setChapter1SightRadialBarrageState(2, isAttackState);
	}

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

	void AvoidanceManager::drawChapter1EndingFade() const {
		if (activeChapter != 1) {
			return;
		}

		const auto& settings = chapter1EndingFadeSettings;
		const int32 localStep = step - Global::startStep_Chapter1;
		if (localStep < settings.startStep) {
			return;
		}

		const int32 durationStep = Max(settings.endStep - settings.startStep, 1);
		const double t = (localStep - settings.startStep) / static_cast<double>(durationStep);
		ColorF fadeColor = settings.color;
		fadeColor.a *= applyEasing(EasingMoveType::EaseInOut, t);
		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(fadeColor);
	}

	void AvoidanceManager::drawChapter1SniperSights() const {
		if (activeChapter != 1) {
			return;
		}

		const auto& settings = chapter1SniperSightSettings;
		const int32 localStep = step - Global::startStep_Chapter1;
		double alphaRate = 1.0;
		if (settings.fadeOutStartStep <= localStep) {
			const int32 fadeAge = localStep - settings.fadeOutStartStep;
			if (settings.fadeOutStep <= fadeAge) {
				return;
			}

			const double t = fadeAge / static_cast<double>(Max(settings.fadeOutStep, 1));
			alphaRate = 1.0 - applyEasing(EasingMoveType::EaseInOut, t);
		}

		for (int32 i = 0; i < static_cast<int32>(chapter1SniperSightSquareIndices.size()); ++i) {
			const Vec2 center = getChapter1SightCenter(i, localStep);
			const ColorF centerColor = settings.centerColors[i % settings.centerColors.size()];
			const double sizeRate = getChapter1SightPulseSizeRate(i, localStep);
			drawChapter1SniperSightAt(center, centerColor, sizeRate, localStep, alphaRate);
		}
	}

	// step :0 - 839
	void AvoidanceManager::chapter1() {
		Timeline timeline(previousStep, step);

		for (const auto& controller : chapter1SightRadialBarrageControllers) {
			if (controller) {
				controller->update(chapter1RedSightRadialBarrageSettings);
			}
		}

		timeline.at(Global::startStep_Chapter1, [&] {
			chapter1MotionStopStep = -1;
			chapter1SightRadialBarrageDeleteStartStep = -1;
			initializeChapter1RedSightRadialBarrageController();
			createChapter1GiantGreenCherry(*this);
			createChapter1GreenRadarLines(*this);
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

		timeline.at(Global::startStep_Chapter1 + 124, [&] {
			createChapter1SightBarrages(*this, 124);
			startChapter1FrameFall();
			startChapter1SightMoveToInnerRing(124);
		});

		for (int32 sightIndex = 0; sightIndex < chapter1SniperSightSettings.sightCount; ++sightIndex) {
			const int32 sightStartStep = chapter1RedSightRadialBarrageSettings.sightStartSteps[
				sightIndex % chapter1RedSightRadialBarrageSettings.sightStartSteps.size()];
			timeline.every(
				chapter1RedSightRadialBarrageSettings.repeatIntervalStep,
				Global::startStep_Chapter1 + sightStartStep,
				Global::startStep_Chapter1 + 317,
				[&, sightIndex, sightStartStep](int32 repeatStep) {
					createChapter1SightRadialBarrage(*this, sightIndex, sightStartStep + repeatStep);
				});
		}

		timeline.at(Global::startStep_Chapter1 + 198, [&] {
			requestChapter1RedSightRadialBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter1 + 260, [&] {
			requestChapter1BlueSightRadialBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter1 + chapter1BlackRingBarrageSettings.startStep, [&] {
			createChapter1BlackRingBarrage(*this);
		});

		timeline.at(Global::startStep_Chapter1 + 318, [&] {
			stopChapter1Motion(318);
			requestChapter1GreenSightRadialBarrageState(false);
			requestChapter1RedSightRadialBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter1 + 326, [&] {
			requestChapter1RedSightRadialBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter1 + 340, [&] {
			requestChapter1BlueSightRadialBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter1 + 348, [&] {
			requestChapter1BlueSightRadialBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter1 + 362, [&] {
			requestChapter1GreenSightRadialBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter1 + 370, [&] {
			requestChapter1GreenSightRadialBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter1 + 380, [&] {
			chapter1SightRadialBarrageDeleteStartStep = 380;
			createChapter1GreenRadarSweepLine(*this);
		});

		const int32 sonarRoundStep = Max(
			chapter1GreenRadarSweepSettings.oneRoundEndStep - chapter1GreenRadarSweepSettings.startStep,
			1);
		const auto signalSettings = getChapter1GreenRadarSignalSettings();
		const int32 normalRoundCount = Min(signalSettings.roundCount, 3);
		for (int32 roundIndex = 0; roundIndex < normalRoundCount; ++roundIndex) {
			const int32 roundStartStep = chapter1GreenRadarSweepSettings.startStep + sonarRoundStep * roundIndex;
			const int32 roundEndStep = roundStartStep + sonarRoundStep;
			timeline.at(Global::startStep_Chapter1 + roundStartStep, [&, roundStartStep, roundEndStep] {
				createChapter1GreenRadarSignals(
					*this,
					roundStartStep,
					roundEndStep,
					roundEndStep,
					false);
			});
		}

		if (4 <= signalSettings.roundCount) {
			timeline.at(Global::startStep_Chapter1 + chapter1GreenRadarSweepSettings.fourthRoundStartStep, [&] {
				createChapter1GreenRadarSignals(
					*this,
					chapter1GreenRadarSweepSettings.fourthRoundStartStep,
					chapter1GreenRadarSweepSettings.fourthRoundSweepEndStep,
					chapter1GreenRadarSweepSettings.fourthRoundAttackStep,
					true);
			});
		}

		timeline.every(
			chapter1GreenRadarSweepSettings.trailIntervalStep,
			Global::startStep_Chapter1 + chapter1GreenRadarSweepSettings.startStep,
			Global::startStep_Chapter1 + chapter1GreenRadarSweepSettings.trailEndStep,
			[&](int32 sweepAge) {
				createChapter1GreenRadarSweepTrail(
					*this,
					chapter1GreenRadarSweepSettings.startStep + sweepAge);
			});
	}
}
