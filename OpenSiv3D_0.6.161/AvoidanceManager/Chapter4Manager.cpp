#include "AvoidanceManager.h"

namespace {
	struct Chapter4OpeningAppleSettings {
		String textureName = U"sprCherryWhite";
		ColorF color = Palette::White;
		int32 moveTime = 8;
		int32 spreadMoveTime = 14;
		int32 fadeInTime = 8;
		Array<int32> spreadStartSteps = { 15, 30, 35 };
		int32 targetPointCount = 24;
		double spreadRadius = 112.0;
		int32 flashStep = 50;
		int32 symbolClearFlashStep = 163;
		int32 flashDurationStep = 10;
		int32 ringCount = 12;
		int32 ringShrinkStep = 35;
		double ringStartRadius = 360.0;
		double ringTargetRadius = 100.0;
		double ringRotationDegrees = 180.0;
		double ringScale = 2;
		int32 cameraZoomStep = 45;
		double cameraTargetScale = 2.5;
		int32 redLineStep = 100;
		int32 redLineFadeOutStep = 18;
		double redLineSpacing = 16.0;
		double redLineScale = 1.0;
		double secondRedLineLength = 180.0;
		double secondRedLineSpacing = 20.0;
		double secondRedLineScale = 1.0;
		double secondRedLineOffscreenMargin = 40.0;
		double secondRedLineLeftX = 270.0;
		double secondRedLineRightX = 560.0;
		double secondRedLineTopY = 138.0;
		double secondRedLineBottomY = 528.0;
		double secondRedLineBlowMinSpeed = 5.5;
		double secondRedLineBlowMaxSpeed = 7.0;
		double secondRedLineBlowGravity = 0.25;
		double secondRedLineBlowMinRotationSpeed = 0.08;
		double secondRedLineBlowMaxRotationSpeed = 0.18;
		int32 radialDecorRingCount = 3;
		int32 radialDecorCherryCount = 36;
		double radialDecorBaseSpeed = 5.6;
		double radialDecorSpeedStep = 2.2;
		double radialDecorScale = 1.0;
		double radialDecorAlpha = 0.42;
		double radialDecorDepth = Iwanna::DrawDepth::Block - 0.5;
		int32 sightBarrageStep = 235;
		int32 sightFadeInStep = 30;
		int32 sightOuterCount = 36;
		int32 sightFocusStep = 360;
		int32 sightReverseStopStep = 404;
		int32 sightCrossAttackStep = 405;
		int32 sightCrossReturnFadeStep = 12;
		int32 sightShrinkStep = sightFocusStep - sightBarrageStep;
		double sightStartRadius = 1220.0;
		double sightRadius = 72.0;
		double sightCrossExtend = 12.0;
		double sightCrossSpacing = 2.0;
		double sightOuterScale = 0.5;
		double sightCrossScale = 0.2;
		double sightCenterScale = 0.55;
		double sightCrossAlpha = 0.65;
		double sightCandidateMinY = 208.0;
		double sightCandidateMaxX = 500.0;
		double sightRotationSpeed = 4.0;
		double sightMaskDepth = Iwanna::DrawDepth::Cherry + 3.0;
		double sightMaskExtraRadius = 0.0;
		double sightDepth = Iwanna::DrawDepth::Cherry + 4.0;
		int32 secondAppleFlowStep = 118;
		double secondAppleVerticalSpacing = 128.0;
		int32 symbolExpansionStep = 10;
		int32 symbolFadeOutStep = 15;
		int32 symbolCircleCount = 36;
		double symbolRadius = 56.0;
		double symbolLineLength = 132.0;
		double symbolLineSpacing = 12.0;
		double symbolScale = 0.75;
		int32 largeAppleMoveEndStep = 205;
		double largeAppleMoveDistance = 250.0;
		double largeAppleAlpha = 0.5;
		double largeAppleScale = 5.0;
		int32 largeAppleActivateStep = 210;
		int32 largeAppleFadeInStep = 10;
		int32 largeAppleDashStep = 225;
		int32 largeAppleDashMoveStep = 10;
		int32 largeAppleTrailIntervalStep = 1;
		int32 largeAppleTrailFadeOutStep = 10;
		double largeAppleTrailAlpha = 0.32;
		double largeAppleDashShakeAmplitude = 6.0;
		int32 largeAppleDashShakeDurationStep = 18;
		double largeAppleDashShakeFrequency = 0.85;
		int32 largeAppleFallStep = 240;
		double largeAppleFallMinHorizontalSpeed = 0.6;
		double largeAppleFallMaxHorizontalSpeed = 1.8;
		double largeAppleFallMinDownSpeed = 0.4;
		double largeAppleFallMaxDownSpeed = 1.2;
		double largeAppleFallGravity = 0.45;
		double largeAppleFallMinRotationSpeed = 0.08;
		double largeAppleFallMaxRotationSpeed = 0.18;
		ColorF largeAppleFallColor = Palette::Gray;
		double scale = 2.0;
		double depth = Iwanna::DrawDepth::Player + 11.0;
	};

	const Chapter4OpeningAppleSettings chapter4OpeningAppleSettings{};
	Array<std::shared_ptr<Iwanna::Cherry>> chapter4OpeningApples;
	Array<std::shared_ptr<Iwanna::Cherry>> chapter4OpeningWhiteRingApples;
	Array<std::shared_ptr<Iwanna::Cherry>> chapter4OpeningSymbolApples;
	Array<std::shared_ptr<Iwanna::Cherry>> chapter4OpeningLargeApples;
	Array<std::shared_ptr<Iwanna::Cherry>> chapter4SecondRedLineApples;
	Array<bool> chapter4OpeningSymbolIsCircle;
	bool chapter4OpeningBlackoutVisible = true;
	int32 chapter4OpeningFlashStartStep = -1;
	bool chapter4OpeningCameraZoomEnabled = false;
	bool chapter4SightCameraZoomEnabled = false;
	int32 chapter4SightCameraStartStep = -1;
	Vec2 chapter4SightCameraTargetCenter{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
	bool chapter4SightOuterMaskVisible = false;
	double chapter4OpeningBaseAngle = -90.0;
	double chapter4OpeningRingRotationDirection = 1.0;

	Array<std::pair<Vec2, Vec2>> getChapter4OpeningAppleRoutes() {
		return {
			{ Vec2{ 240, 350 }, Vec2{ 240, 304 } },
			{ Vec2{ 400, 250.0 }, Vec2{ 400, 304 } },
			{ Vec2{ 560, 350 }, Vec2{ 560, 304 } },
		};
	}

	Array<Vec2> getChapter4SecondAppleSpreadTargets() {
		const auto& settings = chapter4OpeningAppleSettings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		return {
			center + Vec2{ 0.0, -settings.secondAppleVerticalSpacing },
			center,
			center + Vec2{ 0.0, settings.secondAppleVerticalSpacing },
		};
	}

	Vec2 calculateChapter4OpeningApplePos(
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		int32 moveTime,
		int32 spreadStartStep,
		int32 spreadMoveTime,
		int32 age) {

		if (age < spreadStartStep) {
			const double t = age / static_cast<double>(Max(moveTime, 1));
			const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			return startPos + (targetPos - startPos) * eased;
		}

		const double t = (age - spreadStartStep) / static_cast<double>(Max(spreadMoveTime, 1));
		const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
		return targetPos + (spreadTargetPos - targetPos) * eased;
	}

	Iwanna::Cherry::Behavior makeChapter4OpeningAppleBehavior(
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		int32 moveTime,
		int32 spreadStartStep,
		int32 spreadMoveTime,
		bool hideOnSpreadStart) {

		return [startPos, targetPos, spreadTargetPos, moveTime, spreadStartStep, spreadMoveTime, hideOnSpreadStart](Iwanna::Cherry& self, int32 age) {
			if (age < spreadStartStep) {
				self.pos = calculateChapter4OpeningApplePos(
					startPos,
					targetPos,
					spreadTargetPos,
					moveTime,
					spreadStartStep,
					spreadMoveTime,
					age);
				return;
			}

			if (hideOnSpreadStart) {
				self.alpha = 0.0;
			}

			self.pos = calculateChapter4OpeningApplePos(
				startPos,
				targetPos,
				spreadTargetPos,
				moveTime,
				spreadStartStep,
				spreadMoveTime,
				age);
		};
	}

	Iwanna::Cherry::Behavior makeChapter4OpeningRingBehavior(
		double angle,
		double startRadius,
		double targetRadius,
		double rotationDirection,
		double rotationDegrees,
		int32 shrinkStep) {

		return [angle, startRadius, targetRadius, rotationDirection, rotationDegrees, shrinkStep](Iwanna::Cherry& self, int32 age) {
			const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
			const double t = age / static_cast<double>(Max(shrinkStep, 1));
			const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			const double radius = startRadius + (targetRadius - startRadius) * eased;
			const double currentAngle = angle + rotationDirection * rotationDegrees * eased;
			self.pos = center + Vec2{
				Math::Cos(Math::ToRadians(currentAngle)) * radius,
				Math::Sin(Math::ToRadians(currentAngle)) * radius,
			};
			self.canPlayerKill = true;
		};
	}

	Iwanna::Cherry::Behavior makeChapter4RedLineBehavior(int32 fadeOutStep) {
		return [fadeOutStep](Iwanna::Cherry& self, int32 age) {
			self.canPlayerKill = (age == 0);

			if (0 < age) {
				const double t = (age - 1) / static_cast<double>(Max(fadeOutStep, 1));
				self.alpha = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			}

			if (fadeOutStep < age) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter4SecondRedLineMoveInBehavior(
		const Vec2& startPos,
		const Vec2& targetPos,
		int32 moveStep) {

		return [startPos, targetPos, moveStep](Iwanna::Cherry& self, int32 age) {
			const double t = age / static_cast<double>(Max(moveStep, 1));
			const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
			self.pos = startPos + (targetPos - startPos) * eased;
			self.canPlayerKill = true;
		};
	}

	Iwanna::Cherry::Behavior makeChapter4SecondRedLineBlowBehavior(
		const Vec2& startPos,
		int32 blowStartAge,
		const Vec2& velocity,
		double gravity,
		double rotationSpeed) {

		return [startPos, blowStartAge, velocity, gravity, rotationSpeed](Iwanna::Cherry& self, int32 age) {
			const double blowAge = Max(age - blowStartAge, 0);
			self.pos = startPos + velocity * blowAge + Vec2{ 0.0, 0.5 * gravity * blowAge * blowAge };
			self.textureAngle = rotationSpeed * blowAge;
			self.canPlayerKill = true;
		};
	}

	Iwanna::Cherry::Behavior makeChapter4LargeAppleBehavior(
		const Vec2& startPos,
		double moveDirection,
		double moveDistance,
		double alpha,
		int32 moveStep,
		bool isCrossSource) {

		return [startPos, moveDirection, moveDistance, alpha, moveStep, isCrossSource](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter4OpeningAppleSettings;
			const int32 activateStep = settings.largeAppleActivateStep - settings.symbolClearFlashStep;
			const int32 dashStep = settings.largeAppleDashStep - settings.symbolClearFlashStep;
			const int32 dashMoveStep = Max(settings.largeAppleDashMoveStep, 1);
			const double firstMoveT = Min(age, moveStep) / static_cast<double>(Max(moveStep, 1));
			const double firstMoveEased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, firstMoveT);
			const Vec2 firstMovePos = startPos + Vec2{ moveDirection * moveDistance * firstMoveEased, 0.0 };
			const Vec2 dashStartPos = startPos + Vec2{ moveDirection * moveDistance, 0.0 };
			const Vec2 dashTargetPos{ Global::windowWidth - dashStartPos.x, dashStartPos.y };

			if (age < dashStep) {
				self.pos = firstMovePos;
			}
			else {
				const double dashT = (age - dashStep) / static_cast<double>(dashMoveStep);
				const double dashEased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, dashT);
				self.pos = dashStartPos + (dashTargetPos - dashStartPos) * dashEased;
			}

			self.textureAngle = moveDirection * Math::ToRadians(360.0) * firstMoveEased;
			self.canPlayerKill = isCrossSource && (activateStep <= age);

			if (isCrossSource && activateStep <= age) {
				const double fadeT = (age - activateStep) / static_cast<double>(Max(settings.largeAppleFadeInStep, 1));
				const double fadeEased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, fadeT);
				self.alpha = alpha + (1.0 - alpha) * fadeEased;
			}
			else {
				self.alpha = alpha;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter4LargeAppleTrailBehavior(double initialAlpha) {
		return [initialAlpha](Iwanna::Cherry& self, int32 age) {
			const auto& settings = chapter4OpeningAppleSettings;
			const double t = age / static_cast<double>(Max(settings.largeAppleTrailFadeOutStep, 1));
			self.canPlayerKill = false;
			self.alpha = initialAlpha * (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t));

			if (settings.largeAppleTrailFadeOutStep <= age) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter4LargeAppleFallBehavior(
		const Vec2& startPos,
		double startAngle,
		int32 fallStartAge,
		double horizontalSpeed,
		double downSpeed,
		double gravity,
		double rotationSpeed,
		bool canKillPlayer) {

		return [startPos, startAngle, fallStartAge, horizontalSpeed, downSpeed, gravity, rotationSpeed, canKillPlayer](Iwanna::Cherry& self, int32 age) {
			const double fallAge = Max(age - fallStartAge, 0);
			self.pos = startPos + Vec2{
				horizontalSpeed * fallAge,
				downSpeed * fallAge + 0.5 * gravity * fallAge * fallAge,
			};
			self.textureAngle = startAngle + rotationSpeed * fallAge;
			self.canPlayerKill = canKillPlayer;
		};
	}

	Iwanna::Cherry::Behavior makeChapter4SymbolBurstBehavior(
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		const Vec2& targetOffset,
		int32 moveTime,
		int32 spreadStartStep,
		int32 spreadMoveTime,
		int32 expansionStep,
		int32 fadeOutStep,
		int32 startDelayStep,
		bool fadeOut) {

		return [startPos, targetPos, spreadTargetPos, targetOffset, moveTime, spreadStartStep, spreadMoveTime, expansionStep, fadeOutStep, startDelayStep, fadeOut](Iwanna::Cherry& self, int32 age) {
			const Vec2 center = calculateChapter4OpeningApplePos(
				startPos,
				targetPos,
				spreadTargetPos,
				moveTime,
				spreadStartStep,
				spreadMoveTime,
				age);

			if (age < startDelayStep) {
				self.pos = center;
				self.alpha = 0.0;
				self.canPlayerKill = false;
				return;
			}

			const int32 burstAge = age - startDelayStep;
			if (burstAge <= expansionStep) {
				const double t = burstAge / static_cast<double>(Max(expansionStep, 1));
				const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
				self.pos = center + targetOffset * eased;
				self.canPlayerKill = false;
				self.alpha = 1.0;
				return;
			}

			self.pos = center + targetOffset;
			self.canPlayerKill = false;

			if (!fadeOut) {
				self.alpha = 1.0;
				return;
			}

			const int32 fadeAge = burstAge - expansionStep;
			const double t = fadeAge / static_cast<double>(Max(fadeOutStep, 1));
			self.alpha = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);

			if (fadeOutStep <= fadeAge) {
				self.isDelete = true;
			}
		};
	}

	void createChapter4SymbolBurstCherry(
		Iwanna::AvoidanceManager& manager,
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		const Vec2& targetOffset,
		int32 moveTime = 1,
		int32 spreadStartStep = 0,
		int32 spreadMoveTime = 1,
		int32 startDelayStep = 0,
		bool fadeOut = true) {

		const auto& settings = chapter4OpeningAppleSettings;
		Iwanna::Cherry::Settings cherrySettings{
			.textureName = settings.textureName,
			.color = settings.color,
			.behavior = makeChapter4SymbolBurstBehavior(
				startPos,
				targetPos,
				spreadTargetPos,
				targetOffset,
				moveTime,
				spreadStartStep,
				spreadMoveTime,
				settings.symbolExpansionStep,
				settings.symbolFadeOutStep,
				startDelayStep,
				fadeOut),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = false,
			.depth = settings.depth + 0.2,
			.scale = settings.symbolScale,
			.canPlayerKillBeforeFullAlpha = false,
			.manualCanPlayerKillControl = true,
		};
		auto apple = std::make_shared<Iwanna::Cherry>();
		apple->reset(startPos, cherrySettings);
		chapter4OpeningSymbolApples << apple;
		manager.createCherry(apple);
	}

	void createChapter4CircleBurst(Iwanna::AvoidanceManager& manager, const Vec2& center, int32 startDelayStep = 0, bool fadeOut = true) {
		const auto& settings = chapter4OpeningAppleSettings;
		const int32 count = Max(settings.symbolCircleCount, 1);

		for (int32 i = 0; i < count; ++i) {
			const double angle = 360.0 * i / count;
			createChapter4SymbolBurstCherry(manager, center, center, center, Vec2{
				Math::Cos(Math::ToRadians(angle)) * settings.symbolRadius,
				Math::Sin(Math::ToRadians(angle)) * settings.symbolRadius,
			}, 1, 0, 1, startDelayStep, fadeOut);
		}
	}

	void createChapter4CrossBurst(Iwanna::AvoidanceManager& manager, const Vec2& center, int32 startDelayStep = 0, bool fadeOut = true) {
		const auto& settings = chapter4OpeningAppleSettings;
		const int32 count = Max(static_cast<int32>(std::ceil(settings.symbolLineLength / settings.symbolLineSpacing)) + 1, 2);
		const double halfLength = settings.symbolLineLength / 2.0;

		for (const double lineAngle : { 45.0, 135.0 }) {
			const Vec2 direction{
				Math::Cos(Math::ToRadians(lineAngle)),
				Math::Sin(Math::ToRadians(lineAngle)),
			};

			for (int32 i = 0; i < count; ++i) {
				const double offset = -halfLength + settings.symbolLineLength * i / static_cast<double>(count - 1);
				createChapter4SymbolBurstCherry(manager, center, center, center, direction * offset, 1, 0, 1, startDelayStep, fadeOut);
			}
		}
	}

	void createChapter4MovingCircleBurst(
		Iwanna::AvoidanceManager& manager,
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		int32 startDelayStep) {

		const auto& settings = chapter4OpeningAppleSettings;
		const int32 count = Max(settings.symbolCircleCount, 1);

		for (int32 i = 0; i < count; ++i) {
			const double angle = 360.0 * i / count;
			createChapter4SymbolBurstCherry(
				manager,
				startPos,
				targetPos,
				spreadTargetPos,
				Vec2{
					Math::Cos(Math::ToRadians(angle)) * settings.symbolRadius,
					Math::Sin(Math::ToRadians(angle)) * settings.symbolRadius,
				},
				settings.moveTime,
				startDelayStep,
				settings.spreadMoveTime,
				startDelayStep,
				false);
		}
	}

	void createChapter4MovingCrossBurst(
		Iwanna::AvoidanceManager& manager,
		const Vec2& startPos,
		const Vec2& targetPos,
		const Vec2& spreadTargetPos,
		int32 startDelayStep) {

		const auto& settings = chapter4OpeningAppleSettings;
		const int32 count = Max(static_cast<int32>(std::ceil(settings.symbolLineLength / settings.symbolLineSpacing)) + 1, 2);
		const double halfLength = settings.symbolLineLength / 2.0;

		for (const double lineAngle : { 45.0, 135.0 }) {
			const Vec2 direction{
				Math::Cos(Math::ToRadians(lineAngle)),
				Math::Sin(Math::ToRadians(lineAngle)),
			};

			for (int32 i = 0; i < count; ++i) {
				const double offset = -halfLength + settings.symbolLineLength * i / static_cast<double>(count - 1);
				createChapter4SymbolBurstCherry(
					manager,
					startPos,
					targetPos,
					spreadTargetPos,
					direction * offset,
					settings.moveTime,
					startDelayStep,
					settings.spreadMoveTime,
					startDelayStep,
					false);
			}
		}
	}

	void createChapter4CircleOrCrossBurst(Iwanna::AvoidanceManager& manager) {
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		if (Random(0, 1) == 0) {
			createChapter4CircleBurst(manager, center);
		}
		else {
			createChapter4CrossBurst(manager, center);
		}
	}

	void createChapter4AppleSymbolBursts(
		Iwanna::AvoidanceManager& manager,
		const Array<std::pair<Vec2, Vec2>>& appleRoutes,
		const Array<Vec2>& spreadTargets) {

		const auto& settings = chapter4OpeningAppleSettings;
		const int32 circleIndex = Random(static_cast<int32>(appleRoutes.size()) - 1);
		chapter4OpeningSymbolIsCircle.clear();

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const Vec2 center = appleRoutes[i].second;
			const int32 startDelayStep = settings.spreadStartSteps[i];
			const bool isCircle = (i == circleIndex);
			chapter4OpeningSymbolIsCircle << isCircle;

			if (isCircle) {
				createChapter4MovingCircleBurst(manager, appleRoutes[i].first, center, spreadTargets[i], startDelayStep);
			}
			else {
				createChapter4MovingCrossBurst(manager, appleRoutes[i].first, center, spreadTargets[i], startDelayStep);
			}
		}
	}

	void createChapter4OpeningWhiteRing(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4OpeningAppleSettings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const int32 ringCount = Max(settings.ringCount, 1);
		chapter4OpeningRingRotationDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;

		for (int32 i = 0; i < ringCount; ++i) {
			const double angle = chapter4OpeningBaseAngle + 360.0 * i / ringCount;
			const Vec2 startPos = center + Vec2{
				Math::Cos(Math::ToRadians(angle)) * settings.ringStartRadius,
				Math::Sin(Math::ToRadians(angle)) * settings.ringStartRadius,
			};

			Iwanna::Cherry::Settings cherrySettings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter4OpeningRingBehavior(
					angle,
					settings.ringStartRadius,
					settings.ringTargetRadius,
					chapter4OpeningRingRotationDirection,
					settings.ringRotationDegrees,
					settings.ringShrinkStep),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = true,
				.depth = settings.depth,
				.scale = settings.ringScale,
				.canPlayerKillBeforeFullAlpha = true,
				.manualCanPlayerKillControl = true,
			};
			auto apple = std::make_shared<Iwanna::Cherry>();
			apple->reset(startPos, cherrySettings);
			chapter4OpeningWhiteRingApples << apple;
			manager.createCherry(apple);
		}
	}

	void deleteChapter4OpeningWhiteRing() {
		for (const auto& apple : chapter4OpeningWhiteRingApples) {
			if (apple) {
				apple->isDelete = true;
			}
		}

		chapter4OpeningWhiteRingApples.clear();
	}

	void deleteChapter4OpeningSymbols() {
		for (const auto& apple : chapter4OpeningSymbolApples) {
			if (apple) {
				apple->isDelete = true;
			}
		}

		chapter4OpeningSymbolApples.clear();
	}

	void deleteChapter4OpeningLargeApples() {
		for (const auto& apple : chapter4OpeningLargeApples) {
			if (apple) {
				apple->isDelete = true;
			}
		}

		chapter4OpeningLargeApples.clear();
	}

	void deleteChapter4SecondRedLineApples() {
		for (const auto& apple : chapter4SecondRedLineApples) {
			if (apple) {
				apple->isDelete = true;
			}
		}

		chapter4SecondRedLineApples.clear();
	}

	void createChapter4LargeAppleTrail(Iwanna::AvoidanceManager& manager, const Iwanna::Cherry& source) {
		const auto& settings = chapter4OpeningAppleSettings;
		auto trail = std::make_shared<Iwanna::Cherry>();
		trail->reset(source.pos, Iwanna::Cherry::Settings{
			.textureName = settings.textureName,
			.color = settings.color,
			.behavior = makeChapter4LargeAppleTrailBehavior(settings.largeAppleTrailAlpha),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = false,
			.depth = settings.depth,
			.scale = settings.largeAppleScale,
			.alpha = settings.largeAppleTrailAlpha,
			.canPlayerKillBeforeFullAlpha = false,
			.manualCanPlayerKillControl = true,
		});
		trail->textureAngle = source.textureAngle;
		manager.createCherry(trail);
	}

	void createChapter4LargeAppleTrails(Iwanna::AvoidanceManager& manager) {
		for (const auto& apple : chapter4OpeningLargeApples) {
			if (apple && !apple->isDelete) {
				createChapter4LargeAppleTrail(manager, *apple);
			}
		}
	}

	void createChapter4SecondRedLineApples(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4OpeningAppleSettings;
		const int32 moveStep = Max(settings.largeAppleMoveEndStep - settings.symbolClearFlashStep, 1);

		struct RedLineRoute {
			Vec2 begin;
			Vec2 end;
			Vec2 startOffset;
		};

		const Array<RedLineRoute> routes = {
			{
				Vec2{ 0.0, settings.secondRedLineTopY },
				Vec2{ Global::windowWidth, settings.secondRedLineTopY },
				Vec2{ 0.0, -(Global::windowHeight + settings.secondRedLineOffscreenMargin) },
			},
			{
				Vec2{ 0.0, settings.secondRedLineBottomY },
				Vec2{ Global::windowWidth, settings.secondRedLineBottomY },
				Vec2{ 0.0, Global::windowHeight + settings.secondRedLineOffscreenMargin },
			},
			{
				Vec2{ settings.secondRedLineLeftX, 0.0 },
				Vec2{ settings.secondRedLineLeftX, Global::windowHeight },
				Vec2{ -(Global::windowWidth + settings.secondRedLineOffscreenMargin), 0.0 },
			},
			{
				Vec2{ settings.secondRedLineRightX, 0.0 },
				Vec2{ settings.secondRedLineRightX, Global::windowHeight },
				Vec2{ Global::windowWidth + settings.secondRedLineOffscreenMargin, 0.0 },
			},
		};

		deleteChapter4SecondRedLineApples();

		for (const auto& route : routes) {
			const double lineLength = route.begin.distanceFrom(route.end);
			const int32 count = Max(static_cast<int32>(std::ceil(lineLength / settings.secondRedLineSpacing)) + 1, 2);

			for (int32 i = 0; i < count; ++i) {
				const double rate = i / static_cast<double>(count - 1);
				const Vec2 targetPos = route.begin + (route.end - route.begin) * rate;
				const Vec2 startPos = targetPos + route.startOffset;

				auto apple = std::make_shared<Iwanna::Cherry>();
				apple->reset(startPos, Iwanna::Cherry::Settings{
					.textureName = U"sprCherry",
					.color = Palette::Red,
					.behavior = makeChapter4SecondRedLineMoveInBehavior(startPos, targetPos, moveStep),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = true,
					.depth = settings.depth + 0.3,
					.scale = settings.secondRedLineScale,
					.canPlayerKillBeforeFullAlpha = true,
					.manualCanPlayerKillControl = true,
				});
				chapter4SecondRedLineApples << apple;
				manager.createCherry(apple);
			}
		}
	}

	void startChapter4SecondRedLineBlow() {
		const auto& settings = chapter4OpeningAppleSettings;
		const Vec2 screenCenter{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };

		for (const auto& apple : chapter4SecondRedLineApples) {
			if (!apple || apple->isDelete) {
				continue;
			}

			Vec2 direction = apple->pos - screenCenter;
			if (direction.lengthSq() <= 0.0001) {
				const double angle = Random(0.0, Math::TwoPi);
				direction = Vec2{ Math::Cos(angle), Math::Sin(angle) };
			}
			else {
				direction.normalize();
			}

			const double speed = Random(
				settings.secondRedLineBlowMinSpeed,
				settings.secondRedLineBlowMaxSpeed);
			const double rotationSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double rotationSpeed = rotationSign * Random(
				settings.secondRedLineBlowMinRotationSpeed,
				settings.secondRedLineBlowMaxRotationSpeed);

			apple->setBehavior(makeChapter4SecondRedLineBlowBehavior(
				apple->pos,
				apple->getAge(),
				direction * speed,
				settings.secondRedLineBlowGravity,
				rotationSpeed));
		}
	}

	ColorF getChapter4RandomDecorColor() {
		const Array<ColorF> colors = {
			Palette::Red,
			Palette::Orange,
			Palette::Yellow,
			Palette::Lime,
			Palette::Cyan,
			Palette::Dodgerblue,
			Palette::Violet,
			Palette::Hotpink,
		};
		return colors[Random(static_cast<int32>(colors.size()) - 1)];
	}

	void createChapter4RadialDecorationBarrage(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4OpeningAppleSettings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const int32 ringCount = Max(settings.radialDecorRingCount, 1);
		const int32 cherryCount = Max(settings.radialDecorCherryCount, 1);

		for (int32 ring = 0; ring < ringCount; ++ring) {
			const double minSpeed = settings.radialDecorBaseSpeed + settings.radialDecorSpeedStep * ring;
			const double maxSpeed = minSpeed + settings.radialDecorSpeedStep;
			const double angleOffset = Math::ToDegrees(Math::Pi / cherryCount) * ring;

			for (int32 i = 0; i < cherryCount; ++i) {
				const ColorF color = getChapter4RandomDecorColor();
				auto apple = std::make_shared<Iwanna::Cherry>();
				apple->reset(center, Iwanna::Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = color,
					.canDeleteOutOfScreen = true,
					.canPlayerKill = false,
					.depth = settings.radialDecorDepth,
					.scale = settings.radialDecorScale,
					.alpha = settings.radialDecorAlpha,
					.canPlayerKillBeforeFullAlpha = false,
				});
				apple->speed = Random(minSpeed, maxSpeed);
				apple->dir = 360.0 * i / cherryCount + angleOffset;
				manager.createCherry(apple);
			}
		}
	}

	Array<Vec2> getChapter4SightCandidateCenters(Iwanna::AvoidanceManager& manager) {
		constexpr double blockSize = 32.0;
		const auto blocks = manager.getBlocks();
		Array<Point> occupiedCells;
		Array<Vec2> candidates;

		for (const auto& block : blocks) {
			if (!block || !block->getHasCollide()) {
				continue;
			}

			occupiedCells << Point{
				static_cast<int32>(std::round(block->pos.x / blockSize)),
				static_cast<int32>(std::round(block->pos.y / blockSize)),
			};
		}

		for (const auto& block : blocks) {
			if (!block || !block->getHasCollide()) {
				continue;
			}

			const Point cell{
				static_cast<int32>(std::round(block->pos.x / blockSize)),
				static_cast<int32>(std::round(block->pos.y / blockSize)),
			};
			if (cell.y <= 0 || occupiedCells.contains(Point{ cell.x, cell.y - 1 })) {
				continue;
			}

			const Vec2 candidate = block->pos + Vec2{ blockSize / 2.0, -blockSize / 2.0 };
			const auto& settings = chapter4OpeningAppleSettings;
			if (settings.sightCandidateMaxX < candidate.x || candidate.y < settings.sightCandidateMinY) {
				continue;
			}

			candidates << candidate;
		}

		return candidates;
	}

	Vec2 rotateChapter4SightOffset(const Vec2& offset, double rotationRadians) {
		const double c = Math::Cos(rotationRadians);
		const double s = Math::Sin(rotationRadians);
		return Vec2{
			offset.x * c - offset.y * s,
			offset.x * s + offset.y * c,
		};
	}

	double getChapter4SightRotation(double baseSpeed, int32 age, int32 reverseStartAge, int32 reverseDuration) {
		if (age <= reverseStartAge) {
			return baseSpeed * age;
		}

		const double t = Clamp((age - reverseStartAge) / static_cast<double>(Max(reverseDuration, 1)), 0.0, 1.0);
		const double reverseEaseOutDistance = t - t * t + t * t * t / 3.0;
		return baseSpeed * reverseStartAge - baseSpeed * reverseDuration * reverseEaseOutDistance;
	}

	Iwanna::Cherry::Behavior makeChapter4SightBarrageBehavior(
		const Vec2& center,
		const Vec2& targetOffset,
		double baseScale,
		double targetAlpha,
		bool canKill,
		bool shouldScaleVisual,
		bool isCross,
		double rotationDirection) {

		const auto& settings = chapter4OpeningAppleSettings;
		const double targetRadius = Max(settings.sightRadius, 1.0);
		const double startRate = Max(settings.sightStartRadius / targetRadius, 1.0);
		const int32 shrinkStep = Max(settings.sightShrinkStep, 1);
		const int32 fadeStep = Max(settings.sightFadeInStep, 1);
		const double rotationSpeed = Math::ToRadians(settings.sightRotationSpeed) * rotationDirection;
		const int32 reverseStartAge = Max(settings.sightFocusStep - settings.sightBarrageStep, 0);
		const int32 reverseDuration = Max(settings.sightReverseStopStep - settings.sightFocusStep, 1);
		const int32 crossAttackAge = Max(settings.sightCrossAttackStep - settings.sightBarrageStep, 0);
		const int32 crossReturnFadeStep = Max(settings.sightCrossReturnFadeStep, 1);

		return [center, targetOffset, baseScale, targetAlpha, canKill, shouldScaleVisual, isCross, startRate, shrinkStep, fadeStep, rotationSpeed, reverseStartAge, reverseDuration, crossAttackAge, crossReturnFadeStep](Iwanna::Cherry& self, int32 age) {
			const double shrinkT = Clamp(age / static_cast<double>(shrinkStep), 0.0, 1.0);
			const double shrinkRate = startRate + (1.0 - startRate)
				* Iwanna::applyEasing(Iwanna::EasingMoveType::EaseIn, shrinkT);
			const double rotation = getChapter4SightRotation(rotationSpeed, age, reverseStartAge, reverseDuration);
			const Vec2 rotatedOffset = rotateChapter4SightOffset(targetOffset * shrinkRate, rotation);
			const double fadeRate = Iwanna::applyEasing(
				Iwanna::EasingMoveType::EaseInOut,
				Clamp(age / static_cast<double>(fadeStep), 0.0, 1.0));
			ColorF color = Palette::White;
			double alpha = targetAlpha * fadeRate;
			bool currentCanKill = canKill;

			if (isCross && crossAttackAge <= age) {
				const double returnT = Clamp((age - crossAttackAge) / static_cast<double>(crossReturnFadeStep), 0.0, 1.0);
				const double returnRate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, returnT);
				color = ColorF{
					1.0,
					returnRate,
					returnRate,
					1.0,
				};
				alpha = 1.0 + (targetAlpha - 1.0) * returnRate;
				currentCanKill = (age == crossAttackAge);
			}

			self.pos = center + rotatedOffset;
			self.textureAngle = rotation;
			self.setColor(color);
			self.alpha = alpha;
			self.setScale(shouldScaleVisual ? baseScale * shrinkRate : baseScale);
			self.canPlayerKill = currentCanKill;
		};
	}

	void createChapter4SightBarrageCherry(
		Iwanna::AvoidanceManager& manager,
		const Vec2& center,
		const Vec2& targetOffset,
		const ColorF& color,
		double scale,
		bool canKill,
		bool shouldScaleVisual,
		bool isCross,
		double rotationDirection) {

		const auto& settings = chapter4OpeningAppleSettings;
		manager.createCherry(center + targetOffset * (settings.sightStartRadius / Max(settings.sightRadius, 1.0)), Iwanna::Cherry::Settings{
			.textureName = U"sprCherryAllWhite",
			.color = color,
			.behavior = makeChapter4SightBarrageBehavior(
				center,
				targetOffset,
				scale,
				color.a,
				canKill,
				shouldScaleVisual,
				isCross,
				rotationDirection),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = canKill,
			.depth = settings.sightDepth,
			.scale = scale,
			.alpha = 0.0,
			.canPlayerKillBeforeFullAlpha = canKill,
			.manualCanPlayerKillControl = true,
		});
	}

	void createChapter4SightBarrage(Iwanna::AvoidanceManager& manager, int32 currentStep) {
		const auto& settings = chapter4OpeningAppleSettings;
		const auto candidates = getChapter4SightCandidateCenters(manager);
		if (candidates.isEmpty()) {
			return;
		}

		const Vec2 center = candidates[Random(static_cast<int32>(candidates.size()) - 1)];
		const double lineRadius = settings.sightRadius + settings.sightCrossExtend;
		const ColorF outerColor{ 1.0, 1.0, 1.0, 1.0 };
		const ColorF crossColor{ 1.0, 1.0, 1.0, settings.sightCrossAlpha };
		const ColorF centerColor{ 1.0, 0.0, 0.0, 1.0 };
		const double rotationDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;
		chapter4SightCameraZoomEnabled = true;
		chapter4SightCameraStartStep = currentStep;
		chapter4SightCameraTargetCenter = center;

		for (int32 i = 0; i < settings.sightOuterCount; ++i) {
			const double angle = Math::TwoPi * i / Max(settings.sightOuterCount, 1);
			const Vec2 targetOffset{
				Math::Cos(angle) * settings.sightRadius,
				Math::Sin(angle) * settings.sightRadius,
			};
			createChapter4SightBarrageCherry(
				manager,
				center,
				targetOffset,
				outerColor,
				settings.sightOuterScale,
				true,
				true,
				false,
				rotationDirection);
		}

		for (double offset = -lineRadius; offset <= lineRadius; offset += settings.sightCrossSpacing) {
			if (Abs(offset) <= settings.sightCrossSpacing * 0.5) {
				continue;
			}

			createChapter4SightBarrageCherry(
				manager,
				center,
				Vec2{ offset, 0.0 },
				crossColor,
				settings.sightCrossScale,
				false,
				false,
				true,
				rotationDirection);
			createChapter4SightBarrageCherry(
				manager,
				center,
				Vec2{ 0.0, offset },
				crossColor,
				settings.sightCrossScale,
				false,
				false,
				true,
				rotationDirection);
		}

		createChapter4SightBarrageCherry(
			manager,
			center,
			Vec2{ 0.0, 0.0 },
			centerColor,
			settings.sightCenterScale,
			false,
			false,
			false,
			rotationDirection);
	}

	void startChapter4LargeAppleFall() {
		const auto& settings = chapter4OpeningAppleSettings;

		for (const auto& apple : chapter4OpeningLargeApples) {
			if (!apple || apple->isDelete) {
				continue;
			}

			const double horizontalSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double horizontalSpeed = horizontalSign * Random(
				settings.largeAppleFallMinHorizontalSpeed,
				settings.largeAppleFallMaxHorizontalSpeed);
			const double downSpeed = Random(
				settings.largeAppleFallMinDownSpeed,
				settings.largeAppleFallMaxDownSpeed);
			const double rotationSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double rotationSpeed = rotationSign * Random(
				settings.largeAppleFallMinRotationSpeed,
				settings.largeAppleFallMaxRotationSpeed);
			const bool canKillPlayer = apple->canPlayerKill;

			apple->setColor(settings.largeAppleFallColor);
			apple->setBehavior(makeChapter4LargeAppleFallBehavior(
				apple->pos,
				apple->textureAngle,
				apple->getAge(),
				horizontalSpeed,
				downSpeed,
				settings.largeAppleFallGravity,
				rotationSpeed,
				canKillPlayer));
		}
	}

	void createChapter4LargeWhiteApples(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4OpeningAppleSettings;
		const auto appleRoutes = getChapter4OpeningAppleRoutes();
		const auto spreadTargets = getChapter4SecondAppleSpreadTargets();
		const int32 appleAge = settings.symbolClearFlashStep - settings.secondAppleFlowStep;
		const int32 moveStep = Max(settings.largeAppleMoveEndStep - settings.symbolClearFlashStep, 1);

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const double moveDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const Vec2 startPos = calculateChapter4OpeningApplePos(
				appleRoutes[i].first,
				appleRoutes[i].second,
				spreadTargets[i],
				settings.moveTime,
				settings.spreadStartSteps[i],
				settings.spreadMoveTime,
				appleAge);

			auto apple = std::make_shared<Iwanna::Cherry>();
			apple->reset(startPos, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter4LargeAppleBehavior(
					startPos,
					moveDirection,
					settings.largeAppleMoveDistance,
					settings.largeAppleAlpha,
					moveStep,
					(i < static_cast<int32>(chapter4OpeningSymbolIsCircle.size()))
						? !chapter4OpeningSymbolIsCircle[i]
						: true),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = false,
				.depth = settings.depth + 0.1,
				.scale = settings.largeAppleScale,
				.alpha = settings.largeAppleAlpha,
				.canPlayerKillBeforeFullAlpha = false,
				.manualCanPlayerKillControl = true,
			});
			chapter4OpeningLargeApples << apple;
			manager.createCherry(apple);
		}
	}

	void createChapter4RedAppleLines(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4OpeningAppleSettings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const double redLineLength = settings.ringTargetRadius;
		const int32 count = Max(static_cast<int32>(std::ceil(redLineLength / settings.redLineSpacing)) + 1, 2);

		for (int32 lineIndex = 0; lineIndex < 3; ++lineIndex) {
			const double angle = chapter4OpeningBaseAngle + 360.0 * lineIndex / 3.0;
			const Vec2 direction{
				Math::Cos(Math::ToRadians(angle)),
				Math::Sin(Math::ToRadians(angle)),
			};

			for (int32 i = 0; i < count; ++i) {
				const double offset = redLineLength * i / static_cast<double>(count - 1);
				const Vec2 pos = center + direction * offset;

				manager.createCherry(pos, Iwanna::Cherry::Settings{
					.textureName = U"sprCherry",
					.color = Palette::Red,
					.behavior = makeChapter4RedLineBehavior(settings.redLineFadeOutStep),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = true,
					.depth = settings.depth - 0.1,
					.scale = settings.redLineScale,
					.canPlayerKillBeforeFullAlpha = true,
					.manualCanPlayerKillControl = true,
				});
			}
		}
	}
}

namespace Iwanna {
	// step : 1820 - 2259
	void AvoidanceManager::chapter4() {
		Timeline timeline(previousStep, step);

		timeline.at(Global::startStep_Chapter4, [&] {
			createChapter4OpeningAppleFlow();
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.flashStep, [&] {
			for (const auto& apple : chapter4OpeningApples) {
				if (apple) {
					apple->isDelete = true;
				}
			}

			chapter4OpeningApples.clear();
			chapter4OpeningBlackoutVisible = false;
			chapter4OpeningFlashStartStep = step;
			chapter4OpeningCameraZoomEnabled = true;
			createChapter4OpeningWhiteRing(*this);
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.symbolClearFlashStep, [&] {
			deleteChapter4OpeningSymbols();
			chapter4OpeningBlackoutVisible = false;
			chapter4OpeningFlashStartStep = step;
			chapter4OpeningCameraZoomEnabled = false;
			createChapter4LargeWhiteApples(*this);
			createChapter4SecondRedLineApples(*this);
		});

		timeline.every(
			chapter4OpeningAppleSettings.largeAppleTrailIntervalStep,
			Global::startStep_Chapter4 + chapter4OpeningAppleSettings.largeAppleDashStep,
			Global::startStep_Chapter4 + chapter4OpeningAppleSettings.largeAppleDashStep + chapter4OpeningAppleSettings.largeAppleDashMoveStep,
			[&](int32) {
				createChapter4LargeAppleTrails(*this);
			});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.largeAppleDashStep, [&] {
			createChapter4RadialDecorationBarrage(*this);
			createChapter4SightBarrage(*this, step);
			requestScreenShake(
				chapter4OpeningAppleSettings.largeAppleDashShakeAmplitude,
				chapter4OpeningAppleSettings.largeAppleDashShakeDurationStep,
				chapter4OpeningAppleSettings.largeAppleDashShakeFrequency);
			startChapter4SecondRedLineBlow();
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.sightFocusStep, [&] {
			chapter4OpeningFlashStartStep = step;
			chapter4SightOuterMaskVisible = true;
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.largeAppleFallStep, [&] {
			startChapter4LargeAppleFall();
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.redLineStep, [&] {
			createChapter4RadialDecorationBarrage(*this);
			createChapter4RedAppleLines(*this);
		});

		timeline.at(Global::startStep_Chapter4 + chapter4OpeningAppleSettings.secondAppleFlowStep, [&] {
			deleteChapter4OpeningWhiteRing();
			createChapter4OpeningAppleFlow(true);
		});
	}

	void AvoidanceManager::createChapter4OpeningAppleFlow(bool isSecondFlow) {
		const auto& settings = chapter4OpeningAppleSettings;
		deleteChapter4OpeningSymbols();
		deleteChapter4OpeningLargeApples();
		deleteChapter4SecondRedLineApples();
		chapter4OpeningSymbolIsCircle.clear();
		chapter4OpeningApples.clear();
		chapter4OpeningWhiteRingApples.clear();
		chapter4OpeningBlackoutVisible = true;
		chapter4OpeningFlashStartStep = -1;
		chapter4OpeningCameraZoomEnabled = false;
		chapter4SightCameraZoomEnabled = false;
		chapter4SightCameraStartStep = -1;
		chapter4SightCameraTargetCenter = Vec2{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		chapter4SightOuterMaskVisible = false;

		const auto appleRoutes = getChapter4OpeningAppleRoutes();
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		Array<Vec2> spreadTargets;

		if (isSecondFlow) {
			spreadTargets = getChapter4SecondAppleSpreadTargets();
		}
		else {
			const int32 targetPointCount = Max(settings.targetPointCount, 3);
			const int32 targetSpacing = Max(static_cast<int32>(std::ceil(targetPointCount / 3.0)), 1);
			const int32 baseTargetIndex = Random(targetSpacing - 1);
			chapter4OpeningBaseAngle = -90.0 + 360.0 * baseTargetIndex / targetPointCount;

			for (int32 i = 0; i < 3; ++i) {
				const double angle = chapter4OpeningBaseAngle + 360.0 * i / 3.0;
				spreadTargets << center + Vec2{
					Math::Cos(Math::ToRadians(angle)) * settings.spreadRadius,
					Math::Sin(Math::ToRadians(angle)) * settings.spreadRadius,
				};
			}
		}

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const auto& [startPos, targetPos] = appleRoutes[i];
			Cherry::Settings cherrySettings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter4OpeningAppleBehavior(
					startPos,
					targetPos,
					spreadTargets[i],
					settings.moveTime,
					settings.spreadStartSteps[i],
					settings.spreadMoveTime,
					isSecondFlow),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = false,
				.depth = settings.depth,
				.scale = settings.scale,
				.appearanceEffect = CherryEffect::FadeIn,
				.appearanceDuration = settings.fadeInTime,
			};
			auto apple = std::make_shared<Cherry>();
			apple->reset(startPos, cherrySettings);
			chapter4OpeningApples << apple;
			createCherry(apple);
		}

		if (isSecondFlow) {
			createChapter4AppleSymbolBursts(*this, appleRoutes, spreadTargets);
		}
	}

	void AvoidanceManager::drawChapter4OpeningBlackout() const {
		if (activeChapter != 4 || !chapter4OpeningBlackoutVisible) {
			return;
		}

		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(Palette::Black);
	}

	void AvoidanceManager::drawChapter4SightOuterMask() const {
		if (activeChapter != 4 || !chapter4SightOuterMaskVisible) {
			return;
		}

		const auto& settings = chapter4OpeningAppleSettings;
		const double maskRadius = settings.sightRadius + settings.sightCrossExtend + settings.sightMaskExtraRadius;
		const double maskThickness = 2400.0;
		Circle{ chapter4SightCameraTargetCenter, maskRadius + maskThickness / 2.0 }
			.drawFrame(maskThickness, Palette::Black);
	}

	void AvoidanceManager::drawChapter4OpeningFlash() const {
		if (activeChapter != 4 || chapter4OpeningFlashStartStep < 0) {
			return;
		}

		const int32 flashAge = step - chapter4OpeningFlashStartStep;
		const int32 flashDuration = Max(chapter4OpeningAppleSettings.flashDurationStep, 1);
		if (flashDuration < flashAge) {
			return;
		}

		ColorF color = Palette::White;
		color.a = 1.0 - applyEasing(EasingMoveType::EaseOut, flashAge / static_cast<double>(flashDuration));
		Rect{ 0, 0, Global::windowWidth, Global::windowHeight }.draw(color);
	}

	double AvoidanceManager::getChapter4CameraScale() const {
		if (activeChapter != 4) {
			return 1.0;
		}

		const auto& settings = chapter4OpeningAppleSettings;
		if (chapter4SightCameraZoomEnabled && 0 <= chapter4SightCameraStartStep) {
			const int32 zoomAge = step - chapter4SightCameraStartStep;
			const double t = zoomAge / static_cast<double>(Max(settings.sightShrinkStep, 1));
			const double eased = applyEasing(EasingMoveType::EaseIn, t);
			return 1.0 + (settings.cameraTargetScale - 1.0) * eased;
		}

		if (chapter4OpeningFlashStartStep < 0 || !chapter4OpeningCameraZoomEnabled) {
			return 1.0;
		}

		const int32 zoomAge = step - chapter4OpeningFlashStartStep;
		const double t = zoomAge / static_cast<double>(Max(settings.cameraZoomStep, 1));
		const double eased = applyEasing(EasingMoveType::EaseOut, t);
		return 1.0 + (settings.cameraTargetScale - 1.0) * eased;
	}

	Vec2 AvoidanceManager::getChapter4CameraCenter() const {
		const Vec2 screenCenter{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		if (activeChapter != 4 || !chapter4SightCameraZoomEnabled || chapter4SightCameraStartStep < 0) {
			return screenCenter;
		}

		const auto& settings = chapter4OpeningAppleSettings;
		const int32 cameraAge = step - chapter4SightCameraStartStep;
		const double t = cameraAge / static_cast<double>(Max(settings.sightShrinkStep, 1));
		const double eased = applyEasing(EasingMoveType::EaseIn, t);
		return screenCenter + (chapter4SightCameraTargetCenter - screenCenter) * eased;
	}
}
