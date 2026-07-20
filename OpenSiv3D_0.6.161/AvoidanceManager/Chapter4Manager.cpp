#include "AvoidanceManager.h"

namespace {
	struct Chapter4Settings {
		struct Common {
			// 白りんご系で共通して使う見た目と基準レイヤー。
			String whiteAppleTexture = U"sprCherryWhite";
			ColorF whiteAppleColor = Palette::White;
			double whiteAppleScale = 2.0;
			double appleDepth = Iwanna::DrawDepth::Player + 11.0;
		} common;

		struct Timeline {
			// Chapter4開始stepからの相対step。演出の発火タイミングをまとめる。
			int32 firstFlashStep = 50;
			int32 redLineStep = 100;
			int32 secondAppleFlowStep = 118;
			int32 symbolClearFlashStep = 163;
		} timeline;

		struct OpeningApples {
			// 開幕と2回目に生成する3つの白りんごの移動。
			int32 moveTime = 8;
			int32 spreadMoveTime = 14;
			int32 fadeInTime = 8;
			Array<int32> spreadStartSteps = { 15, 30, 35 };
			int32 targetPointCount = 24;
			double firstSpreadRadius = 112.0;
			double secondVerticalSpacing = 128.0;
		} opening;

		struct Flash {
			// 画面全体の白フラッシュ。
			int32 durationStep = 8;
		} flash;

		struct WhiteRing {
			// 1回目フラッシュ後の白りんごリング。
			int32 count = 12;
			int32 shrinkStep = 35;
			double startRadius = 360.0;
			double targetRadius = 100.0;
			double rotationDegrees = 180.0;
			double scale = 2.0;
		} ring;

		struct Camera {
			// 1回目リング演出と照準演出で使うカメラ倍率。
			int32 openingZoomStep = 45;
			double targetScale = 2.5;
		} camera;

		struct RedLine {
			// step1920付近で中心から伸びる赤りんご線。
			int32 fadeOutStep = 18;
			double spacing = 16.0;
			double scale = 1.0;
		} redLine;

		struct SecondRedLine {
			// 2回目フラッシュ後、画面外から入ってくる4本の赤りんご線。
			double spacing = 20.0;
			double scale = 1.0;
			double offscreenMargin = 40.0;
			double leftX = 272.0;
			double rightX = 528.0;
			double topY = 138.0;
			double bottomY = 528.0;
			double blowMinSpeed = 5.5;
			double blowMaxSpeed = 7.0;
			double blowGravity = 0.25;
			double blowMinRotationSpeed = 0.08;
			double blowMaxRotationSpeed = 0.18;
		} secondRedLine;

		struct RadialDecor {
			// 判定なしのカラフル放射状りんご。
			int32 ringCount = 3;
			int32 cherryCount = 36;
			double baseSpeed = 5.6;
			double speedStep = 2.2;
			double scale = 1.0;
			double alpha = 0.42;
			double depth = Iwanna::DrawDepth::Block - 0.5;
		} radialDecor;

		struct Sight {
			// 照準型弾幕。stepはChapter4開始stepからの相対値。
			int32 barrageStep = 235;
			int32 fadeInStep = 30;
			int32 outerCount = 36;
			int32 focusStep = 365;
			int32 reverseStopStep = 409;
			int32 crossAttackStep = 410;
			int32 crossReturnFadeStep = 12;
			int32 shrinkStep = focusStep - barrageStep;
			double startRadius = 1220.0;
			double radius = 72.0;
			double crossExtend = 12.0;
			double crossSpacing = 2.0;
			double outerScale = 0.5;
			double crossScale = 0.2;
			double centerScale = 0.55;
			double crossAlpha = 0.65;
			double candidateMinY = 208.0;
			double candidateMaxX = 500.0;
			double rotationSpeed = 4.0;
			double reverseRotationSpeed = Random<double>(8.0, 12.0);
			int32 maskFadeInStep = 12;
			double maskInnerRadius = 74.0;
			int32 innerMaskFadeInStep = 20;
			int32 cameraZoomOutStep = 85;
			double cameraZoomOutTargetScale = 0.5;
			double depth = Iwanna::DrawDepth::Cherry + 4.0;
		} sight;

		struct Symbol {
			// 2回目の3りんご移動時に出す〇/×。
			int32 expansionStep = 10;
			int32 fadeOutStep = 15;
			int32 circleCount = 36;
			double radius = 56.0;
			double lineLength = 132.0;
			double lineSpacing = 12.0;
			double scale = 0.75;
		} symbol;

		struct LargeApple {
			// 2回目フラッシュ後の巨大白りんご。
			int32 moveEndStep = 205;
			double moveDistance = 250.0;
			double alpha = 0.5;
			double scale = 5.0;
			int32 activateStep = 210;
			int32 fadeInStep = 10;
			int32 dashStep = 225;
			int32 dashMoveStep = 10;
			int32 trailIntervalStep = 1;
			int32 trailFadeOutStep = 10;
			double trailAlpha = 0.32;
			double dashShakeAmplitude = 6.0;
			int32 dashShakeDurationStep = 18;
			double dashShakeFrequency = 0.85;
			int32 fallStep = 240;
			double fallMinHorizontalSpeed = 0.6;
			double fallMaxHorizontalSpeed = 1.8;
			double fallMinDownSpeed = 0.4;
			double fallMaxDownSpeed = 1.2;
			double fallGravity = 0.45;
			double fallMinRotationSpeed = 0.08;
			double fallMaxRotationSpeed = 0.18;
			ColorF fallColor = Palette::Gray;
		} largeApple;
	};

	const Chapter4Settings chapter4Settings{};
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
	int32 chapter4SightOuterMaskStartStep = -1;
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
		const auto& settings = chapter4Settings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		return {
			center + Vec2{ 0.0, -settings.opening.secondVerticalSpacing },
			center,
			center + Vec2{ 0.0, settings.opening.secondVerticalSpacing },
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
			const auto& settings = chapter4Settings;
			const int32 activateStep = settings.largeApple.activateStep - settings.timeline.symbolClearFlashStep;
			const int32 dashStep = settings.largeApple.dashStep - settings.timeline.symbolClearFlashStep;
			const int32 dashMoveStep = Max(settings.largeApple.dashMoveStep, 1);
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
				const double fadeT = (age - activateStep) / static_cast<double>(Max(settings.largeApple.fadeInStep, 1));
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
			const auto& settings = chapter4Settings;
			const double t = age / static_cast<double>(Max(settings.largeApple.trailFadeOutStep, 1));
			self.canPlayerKill = false;
			self.alpha = initialAlpha * (1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t));

			if (settings.largeApple.trailFadeOutStep <= age) {
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

		const auto& settings = chapter4Settings;
		Iwanna::Cherry::Settings cherrySettings{
			.textureName = settings.common.whiteAppleTexture,
			.color = settings.common.whiteAppleColor,
			.behavior = makeChapter4SymbolBurstBehavior(
				startPos,
				targetPos,
				spreadTargetPos,
				targetOffset,
				moveTime,
				spreadStartStep,
				spreadMoveTime,
				settings.symbol.expansionStep,
				settings.symbol.fadeOutStep,
				startDelayStep,
				fadeOut),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = false,
			.depth = settings.common.appleDepth + 0.2,
			.scale = settings.symbol.scale,
			.canPlayerKillBeforeFullAlpha = false,
			.manualCanPlayerKillControl = true,
		};
		auto apple = std::make_shared<Iwanna::Cherry>();
		apple->reset(startPos, cherrySettings);
		chapter4OpeningSymbolApples << apple;
		manager.createCherry(apple);
	}

	void createChapter4CircleBurst(Iwanna::AvoidanceManager& manager, const Vec2& center, int32 startDelayStep = 0, bool fadeOut = true) {
		const auto& settings = chapter4Settings;
		const int32 count = Max(settings.symbol.circleCount, 1);

		for (int32 i = 0; i < count; ++i) {
			const double angle = 360.0 * i / count;
			createChapter4SymbolBurstCherry(manager, center, center, center, Vec2{
				Math::Cos(Math::ToRadians(angle)) * settings.symbol.radius,
				Math::Sin(Math::ToRadians(angle)) * settings.symbol.radius,
			}, 1, 0, 1, startDelayStep, fadeOut);
		}
	}

	void createChapter4CrossBurst(Iwanna::AvoidanceManager& manager, const Vec2& center, int32 startDelayStep = 0, bool fadeOut = true) {
		const auto& settings = chapter4Settings;
		const int32 count = Max(static_cast<int32>(std::ceil(settings.symbol.lineLength / settings.symbol.lineSpacing)) + 1, 2);
		const double halfLength = settings.symbol.lineLength / 2.0;

		for (const double lineAngle : { 45.0, 135.0 }) {
			const Vec2 direction{
				Math::Cos(Math::ToRadians(lineAngle)),
				Math::Sin(Math::ToRadians(lineAngle)),
			};

			for (int32 i = 0; i < count; ++i) {
				const double offset = -halfLength + settings.symbol.lineLength * i / static_cast<double>(count - 1);
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

		const auto& settings = chapter4Settings;
		const int32 count = Max(settings.symbol.circleCount, 1);

		for (int32 i = 0; i < count; ++i) {
			const double angle = 360.0 * i / count;
			createChapter4SymbolBurstCherry(
				manager,
				startPos,
				targetPos,
				spreadTargetPos,
				Vec2{
					Math::Cos(Math::ToRadians(angle)) * settings.symbol.radius,
					Math::Sin(Math::ToRadians(angle)) * settings.symbol.radius,
				},
				settings.opening.moveTime,
				startDelayStep,
				settings.opening.spreadMoveTime,
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

		const auto& settings = chapter4Settings;
		const int32 count = Max(static_cast<int32>(std::ceil(settings.symbol.lineLength / settings.symbol.lineSpacing)) + 1, 2);
		const double halfLength = settings.symbol.lineLength / 2.0;

		for (const double lineAngle : { 45.0, 135.0 }) {
			const Vec2 direction{
				Math::Cos(Math::ToRadians(lineAngle)),
				Math::Sin(Math::ToRadians(lineAngle)),
			};

			for (int32 i = 0; i < count; ++i) {
				const double offset = -halfLength + settings.symbol.lineLength * i / static_cast<double>(count - 1);
				createChapter4SymbolBurstCherry(
					manager,
					startPos,
					targetPos,
					spreadTargetPos,
					direction * offset,
					settings.opening.moveTime,
					startDelayStep,
					settings.opening.spreadMoveTime,
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

		const auto& settings = chapter4Settings;
		const int32 circleIndex = Random(static_cast<int32>(appleRoutes.size()) - 1);
		chapter4OpeningSymbolIsCircle.clear();

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const Vec2 center = appleRoutes[i].second;
			const int32 startDelayStep = settings.opening.spreadStartSteps[i];
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
		const auto& settings = chapter4Settings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const int32 ringCount = Max(settings.ring.count, 1);
		chapter4OpeningRingRotationDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;

		for (int32 i = 0; i < ringCount; ++i) {
			const double angle = chapter4OpeningBaseAngle + 360.0 * i / ringCount;
			const Vec2 startPos = center + Vec2{
				Math::Cos(Math::ToRadians(angle)) * settings.ring.startRadius,
				Math::Sin(Math::ToRadians(angle)) * settings.ring.startRadius,
			};

			Iwanna::Cherry::Settings cherrySettings{
				.textureName = settings.common.whiteAppleTexture,
				.color = settings.common.whiteAppleColor,
				.behavior = makeChapter4OpeningRingBehavior(
					angle,
					settings.ring.startRadius,
					settings.ring.targetRadius,
					chapter4OpeningRingRotationDirection,
					settings.ring.rotationDegrees,
					settings.ring.shrinkStep),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = true,
				.depth = settings.common.appleDepth,
				.scale = settings.ring.scale,
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
		const auto& settings = chapter4Settings;
		auto trail = std::make_shared<Iwanna::Cherry>();
		trail->reset(source.pos, Iwanna::Cherry::Settings{
			.textureName = settings.common.whiteAppleTexture,
			.color = settings.common.whiteAppleColor,
			.behavior = makeChapter4LargeAppleTrailBehavior(settings.largeApple.trailAlpha),
			.canDeleteOutOfScreen = false,
			.canPlayerKill = false,
			.depth = settings.common.appleDepth,
			.scale = settings.largeApple.scale,
			.alpha = settings.largeApple.trailAlpha,
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
		const auto& settings = chapter4Settings;
		const int32 moveStep = Max(settings.largeApple.moveEndStep - settings.timeline.symbolClearFlashStep, 1);

		struct RedLineRoute {
			Vec2 begin;
			Vec2 end;
			Vec2 startOffset;
		};

		const Array<RedLineRoute> routes = {
			{
				Vec2{ 0.0, settings.secondRedLine.topY },
				Vec2{ Global::windowWidth, settings.secondRedLine.topY },
				Vec2{ 0.0, -(Global::windowHeight + settings.secondRedLine.offscreenMargin) },
			},
			{
				Vec2{ 0.0, settings.secondRedLine.bottomY },
				Vec2{ Global::windowWidth, settings.secondRedLine.bottomY },
				Vec2{ 0.0, Global::windowHeight + settings.secondRedLine.offscreenMargin },
			},
			{
				Vec2{ settings.secondRedLine.leftX, 0.0 },
				Vec2{ settings.secondRedLine.leftX, Global::windowHeight },
				Vec2{ -(Global::windowWidth + settings.secondRedLine.offscreenMargin), 0.0 },
			},
			{
				Vec2{ settings.secondRedLine.rightX, 0.0 },
				Vec2{ settings.secondRedLine.rightX, Global::windowHeight },
				Vec2{ Global::windowWidth + settings.secondRedLine.offscreenMargin, 0.0 },
			},
		};

		deleteChapter4SecondRedLineApples();

		for (const auto& route : routes) {
			const double lineLength = route.begin.distanceFrom(route.end);
			const int32 count = Max(static_cast<int32>(std::ceil(lineLength / settings.secondRedLine.spacing)) + 1, 2);

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
					.depth = settings.common.appleDepth + 0.3,
					.scale = settings.secondRedLine.scale,
					.canPlayerKillBeforeFullAlpha = true,
					.manualCanPlayerKillControl = true,
				});
				chapter4SecondRedLineApples << apple;
				manager.createCherry(apple);
			}
		}
	}

	void startChapter4SecondRedLineBlow() {
		const auto& settings = chapter4Settings;
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
				settings.secondRedLine.blowMinSpeed,
				settings.secondRedLine.blowMaxSpeed);
			const double rotationSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double rotationSpeed = rotationSign * Random(
				settings.secondRedLine.blowMinRotationSpeed,
				settings.secondRedLine.blowMaxRotationSpeed);

			apple->setBehavior(makeChapter4SecondRedLineBlowBehavior(
				apple->pos,
				apple->getAge(),
				direction * speed,
				settings.secondRedLine.blowGravity,
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
		const auto& settings = chapter4Settings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const int32 ringCount = Max(settings.radialDecor.ringCount, 1);
		const int32 cherryCount = Max(settings.radialDecor.cherryCount, 1);

		for (int32 ring = 0; ring < ringCount; ++ring) {
			const double minSpeed = settings.radialDecor.baseSpeed + settings.radialDecor.speedStep * ring;
			const double maxSpeed = minSpeed + settings.radialDecor.speedStep;
			const double angleOffset = Math::ToDegrees(Math::Pi / cherryCount) * ring;

			for (int32 i = 0; i < cherryCount; ++i) {
				const ColorF color = getChapter4RandomDecorColor();
				auto apple = std::make_shared<Iwanna::Cherry>();
				apple->reset(center, Iwanna::Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = color,
					.canDeleteOutOfScreen = true,
					.canPlayerKill = false,
					.depth = settings.radialDecor.depth,
					.scale = settings.radialDecor.scale,
					.alpha = settings.radialDecor.alpha,
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
			const auto& settings = chapter4Settings;
			if (settings.sight.candidateMaxX < candidate.x || candidate.y < settings.sight.candidateMinY) {
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

	double getChapter4SightRotation(
		double baseSpeed,
		double reverseSpeed,
		int32 age,
		int32 reverseStartAge,
		int32 reverseDuration) {

		if (age <= reverseStartAge) {
			return baseSpeed * age;
		}

		const double t = Clamp((age - reverseStartAge) / static_cast<double>(Max(reverseDuration, 1)), 0.0, 1.0);
		const double reverseEaseOutDistance = t - t * t + t * t * t / 3.0;
		return baseSpeed * reverseStartAge - reverseSpeed * reverseDuration * reverseEaseOutDistance;
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

		const auto& settings = chapter4Settings;
		const double targetRadius = Max(settings.sight.radius, 1.0);
		const double startRate = Max(settings.sight.startRadius / targetRadius, 1.0);
		const int32 shrinkStep = Max(settings.sight.shrinkStep, 1);
		const int32 fadeStep = Max(settings.sight.fadeInStep, 1);
		const double rotationSpeed = Math::ToRadians(settings.sight.rotationSpeed) * rotationDirection;
		const double reverseRotationSpeed = Math::ToRadians(settings.sight.reverseRotationSpeed) * rotationDirection;
		const int32 reverseStartAge = Max(settings.sight.focusStep - settings.sight.barrageStep - 1, 0);
		const int32 reverseDuration = Max(settings.sight.reverseStopStep - settings.sight.focusStep, 1);
		const int32 crossAttackAge = Max(settings.sight.crossAttackStep - settings.sight.barrageStep, 0);
		const int32 crossReturnFadeStep = Max(settings.sight.crossReturnFadeStep, 1);

		return [center, targetOffset, baseScale, targetAlpha, canKill, shouldScaleVisual, isCross, startRate, shrinkStep, fadeStep, rotationSpeed, reverseRotationSpeed, reverseStartAge, reverseDuration, crossAttackAge, crossReturnFadeStep](Iwanna::Cherry& self, int32 age) {
			const double shrinkT = Clamp(age / static_cast<double>(shrinkStep), 0.0, 1.0);
			const double shrinkRate = startRate + (1.0 - startRate)
				* Iwanna::applyEasing(Iwanna::EasingMoveType::EaseIn, shrinkT);
			const double rotation = getChapter4SightRotation(
				rotationSpeed,
				reverseRotationSpeed,
				age,
				reverseStartAge,
				reverseDuration);
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

		const auto& settings = chapter4Settings;
		manager.createCherry(center + targetOffset * (settings.sight.startRadius / Max(settings.sight.radius, 1.0)), Iwanna::Cherry::Settings{
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
			.depth = settings.sight.depth,
			.scale = scale,
			.alpha = 0.0,
			.canPlayerKillBeforeFullAlpha = canKill,
			.manualCanPlayerKillControl = true,
		});
	}

	void createChapter4SightBarrage(Iwanna::AvoidanceManager& manager, int32 currentStep) {
		const auto& settings = chapter4Settings;
		const auto candidates = getChapter4SightCandidateCenters(manager);
		if (candidates.isEmpty()) {
			return;
		}

		const Vec2 center = candidates[Random(static_cast<int32>(candidates.size()) - 1)];
		const double lineRadius = settings.sight.radius + settings.sight.crossExtend;
		const ColorF outerColor{ 1.0, 1.0, 1.0, 1.0 };
		const ColorF crossColor{ 1.0, 1.0, 1.0, settings.sight.crossAlpha };
		const ColorF centerColor{ 1.0, 0.0, 0.0, 1.0 };
		const double rotationDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;
		chapter4SightCameraZoomEnabled = true;
		chapter4SightCameraStartStep = currentStep;
		chapter4SightCameraTargetCenter = center;

		for (int32 i = 0; i < settings.sight.outerCount; ++i) {
			const double angle = Math::TwoPi * i / Max(settings.sight.outerCount, 1);
			const Vec2 targetOffset{
				Math::Cos(angle) * settings.sight.radius,
				Math::Sin(angle) * settings.sight.radius,
			};
			createChapter4SightBarrageCherry(
				manager,
				center,
				targetOffset,
				outerColor,
				settings.sight.outerScale,
				true,
				true,
				false,
				rotationDirection);
		}

		for (double offset = -lineRadius; offset <= lineRadius; offset += settings.sight.crossSpacing) {
			if (Abs(offset) <= settings.sight.crossSpacing * 0.5) {
				continue;
			}

			createChapter4SightBarrageCherry(
				manager,
				center,
				Vec2{ offset, 0.0 },
				crossColor,
				settings.sight.crossScale,
				false,
				false,
				true,
				rotationDirection);
			createChapter4SightBarrageCherry(
				manager,
				center,
				Vec2{ 0.0, offset },
				crossColor,
				settings.sight.crossScale,
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
			settings.sight.centerScale,
			false,
			false,
			false,
			rotationDirection);
	}

	void startChapter4LargeAppleFall() {
		const auto& settings = chapter4Settings;

		for (const auto& apple : chapter4OpeningLargeApples) {
			if (!apple || apple->isDelete) {
				continue;
			}

			const double horizontalSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double horizontalSpeed = horizontalSign * Random(
				settings.largeApple.fallMinHorizontalSpeed,
				settings.largeApple.fallMaxHorizontalSpeed);
			const double downSpeed = Random(
				settings.largeApple.fallMinDownSpeed,
				settings.largeApple.fallMaxDownSpeed);
			const double rotationSign = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const double rotationSpeed = rotationSign * Random(
				settings.largeApple.fallMinRotationSpeed,
				settings.largeApple.fallMaxRotationSpeed);
			const bool canKillPlayer = apple->canPlayerKill;

			apple->setColor(settings.largeApple.fallColor);
			apple->setBehavior(makeChapter4LargeAppleFallBehavior(
				apple->pos,
				apple->textureAngle,
				apple->getAge(),
				horizontalSpeed,
				downSpeed,
				settings.largeApple.fallGravity,
				rotationSpeed,
				canKillPlayer));
		}
	}

	void createChapter4LargeWhiteApples(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4Settings;
		const auto appleRoutes = getChapter4OpeningAppleRoutes();
		const auto spreadTargets = getChapter4SecondAppleSpreadTargets();
		const int32 appleAge = settings.timeline.symbolClearFlashStep - settings.timeline.secondAppleFlowStep;
		const int32 moveStep = Max(settings.largeApple.moveEndStep - settings.timeline.symbolClearFlashStep, 1);

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const double moveDirection = (Random(0, 1) == 0) ? -1.0 : 1.0;
			const Vec2 startPos = calculateChapter4OpeningApplePos(
				appleRoutes[i].first,
				appleRoutes[i].second,
				spreadTargets[i],
				settings.opening.moveTime,
				settings.opening.spreadStartSteps[i],
				settings.opening.spreadMoveTime,
				appleAge);

			auto apple = std::make_shared<Iwanna::Cherry>();
			apple->reset(startPos, Iwanna::Cherry::Settings{
				.textureName = settings.common.whiteAppleTexture,
				.color = settings.common.whiteAppleColor,
				.behavior = makeChapter4LargeAppleBehavior(
					startPos,
					moveDirection,
					settings.largeApple.moveDistance,
					settings.largeApple.alpha,
					moveStep,
					(i < static_cast<int32>(chapter4OpeningSymbolIsCircle.size()))
						? !chapter4OpeningSymbolIsCircle[i]
						: true),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = false,
				.depth = settings.common.appleDepth + 0.1,
				.scale = settings.largeApple.scale,
				.alpha = settings.largeApple.alpha,
				.canPlayerKillBeforeFullAlpha = false,
				.manualCanPlayerKillControl = true,
			});
			chapter4OpeningLargeApples << apple;
			manager.createCherry(apple);
		}
	}

	void createChapter4RedAppleLines(Iwanna::AvoidanceManager& manager) {
		const auto& settings = chapter4Settings;
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		const double redLineLength = settings.ring.targetRadius;
		const int32 count = Max(static_cast<int32>(std::ceil(redLineLength / settings.redLine.spacing)) + 1, 2);

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
					.behavior = makeChapter4RedLineBehavior(settings.redLine.fadeOutStep),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = true,
					.depth = settings.common.appleDepth - 0.1,
					.scale = settings.redLine.scale,
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

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.timeline.firstFlashStep, [&] {
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

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.timeline.symbolClearFlashStep, [&] {
			deleteChapter4OpeningSymbols();
			chapter4OpeningBlackoutVisible = false;
			chapter4OpeningFlashStartStep = step;
			chapter4OpeningCameraZoomEnabled = false;
			createChapter4LargeWhiteApples(*this);
			createChapter4SecondRedLineApples(*this);
		});

		timeline.every(
			chapter4Settings.largeApple.trailIntervalStep,
			Global::startStep_Chapter4 + chapter4Settings.largeApple.dashStep,
			Global::startStep_Chapter4 + chapter4Settings.largeApple.dashStep + chapter4Settings.largeApple.dashMoveStep,
			[&](int32) {
				createChapter4LargeAppleTrails(*this);
			});

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.largeApple.dashStep, [&] {
			createChapter4RadialDecorationBarrage(*this);
			createChapter4SightBarrage(*this, step);
			requestScreenShake(
				chapter4Settings.largeApple.dashShakeAmplitude,
				chapter4Settings.largeApple.dashShakeDurationStep,
				chapter4Settings.largeApple.dashShakeFrequency);
			startChapter4SecondRedLineBlow();
		});

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.sight.focusStep, [&] {
			chapter4SightOuterMaskVisible = true;
			chapter4SightOuterMaskStartStep = step;
		});

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.largeApple.fallStep, [&] {
			startChapter4LargeAppleFall();
		});

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.timeline.redLineStep, [&] {
			createChapter4RadialDecorationBarrage(*this);
			createChapter4RedAppleLines(*this);
		});

		timeline.at(Global::startStep_Chapter4 + chapter4Settings.timeline.secondAppleFlowStep, [&] {
			deleteChapter4OpeningWhiteRing();
			createChapter4OpeningAppleFlow(true);
		});
	}

	void AvoidanceManager::createChapter4OpeningAppleFlow(bool isSecondFlow) {
		const auto& settings = chapter4Settings;
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
		chapter4SightOuterMaskStartStep = -1;

		const auto appleRoutes = getChapter4OpeningAppleRoutes();
		const Vec2 center{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		Array<Vec2> spreadTargets;

		if (isSecondFlow) {
			spreadTargets = getChapter4SecondAppleSpreadTargets();
		}
		else {
			const int32 targetPointCount = Max(settings.opening.targetPointCount, 3);
			const int32 targetSpacing = Max(static_cast<int32>(std::ceil(targetPointCount / 3.0)), 1);
			const int32 baseTargetIndex = Random(targetSpacing - 1);
			chapter4OpeningBaseAngle = -90.0 + 360.0 * baseTargetIndex / targetPointCount;

			for (int32 i = 0; i < 3; ++i) {
				const double angle = chapter4OpeningBaseAngle + 360.0 * i / 3.0;
				spreadTargets << center + Vec2{
					Math::Cos(Math::ToRadians(angle)) * settings.opening.firstSpreadRadius,
					Math::Sin(Math::ToRadians(angle)) * settings.opening.firstSpreadRadius,
				};
			}
		}

		for (int32 i = 0; i < static_cast<int32>(appleRoutes.size()); ++i) {
			const auto& [startPos, targetPos] = appleRoutes[i];
			Cherry::Settings cherrySettings{
				.textureName = settings.common.whiteAppleTexture,
				.color = settings.common.whiteAppleColor,
				.behavior = makeChapter4OpeningAppleBehavior(
					startPos,
					targetPos,
					spreadTargets[i],
					settings.opening.moveTime,
					settings.opening.spreadStartSteps[i],
					settings.opening.spreadMoveTime,
					isSecondFlow),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = false,
				.depth = settings.common.appleDepth,
				.scale = settings.common.whiteAppleScale,
				.appearanceEffect = CherryEffect::FadeIn,
				.appearanceDuration = settings.opening.fadeInTime,
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

		const auto& settings = chapter4Settings;
		const int32 maskAge = (chapter4SightOuterMaskStartStep < 0)
			? settings.sight.maskFadeInStep
			: step - chapter4SightOuterMaskStartStep;
		const double fadeT = maskAge / static_cast<double>(Max(settings.sight.maskFadeInStep, 1));
		ColorF maskColor = Palette::Black;
		maskColor.a = applyEasing(EasingMoveType::EaseInOut, fadeT);
		const double maskThickness = 2400.0;
		Circle{ chapter4SightCameraTargetCenter, settings.sight.maskInnerRadius + maskThickness / 2.0 }
			.drawFrame(maskThickness, maskColor);
	}

	void AvoidanceManager::drawChapter4SightForeground() const {
		if (activeChapter != 4 || !chapter4SightOuterMaskVisible) {
			return;
		}

		const auto& settings = chapter4Settings;
		for (auto* obj : sortedDrawList) {
			if (obj->type == ObjectType::Cherry
				&& Abs(obj->getDepth() - settings.sight.depth) < 0.001) {
				obj->draw();
			}
		}
	}

	void AvoidanceManager::drawChapter4SightInnerMask() const {
		if (activeChapter != 4 || !chapter4SightOuterMaskVisible) {
			return;
		}

		const auto& settings = chapter4Settings;
		const int32 innerMaskStartStep = Global::startStep_Chapter4 + settings.sight.crossAttackStep;
		if (step < innerMaskStartStep) {
			return;
		}

		const double fadeT = (step - innerMaskStartStep)
			/ static_cast<double>(Max(settings.sight.innerMaskFadeInStep, 1));
		ColorF maskColor = Palette::Black;
		maskColor.a = applyEasing(EasingMoveType::EaseInOut, fadeT);
		Circle{ chapter4SightCameraTargetCenter, 2400.0 }.draw(maskColor);
	}

	void AvoidanceManager::drawChapter4OpeningFlash() const {
		if (activeChapter != 4 || chapter4OpeningFlashStartStep < 0) {
			return;
		}

		const int32 flashAge = step - chapter4OpeningFlashStartStep;
		const int32 flashDuration = Max(chapter4Settings.flash.durationStep, 1);
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

		const auto& settings = chapter4Settings;
		if (chapter4SightCameraZoomEnabled && 0 <= chapter4SightCameraStartStep) {
			const int32 zoomAge = step - chapter4SightCameraStartStep;
			const int32 zoomInStep = Max(settings.sight.shrinkStep, 1);
			if (zoomAge <= zoomInStep) {
				const double t = zoomAge / static_cast<double>(zoomInStep);
				const double eased = applyEasing(EasingMoveType::EaseIn, t);
				return 1.0 + (settings.camera.targetScale - 1.0) * eased;
			}

			const int32 zoomOutStep = Max(settings.sight.cameraZoomOutStep, 1);
			const double t = (zoomAge - zoomInStep) / static_cast<double>(zoomOutStep);
			const double eased = applyEasing(EasingMoveType::EaseInOut, t);
			return settings.camera.targetScale
				+ (settings.sight.cameraZoomOutTargetScale - settings.camera.targetScale) * eased;
		}

		if (chapter4OpeningFlashStartStep < 0 || !chapter4OpeningCameraZoomEnabled) {
			return 1.0;
		}

		const int32 zoomAge = step - chapter4OpeningFlashStartStep;
		const double t = zoomAge / static_cast<double>(Max(settings.camera.openingZoomStep, 1));
		const double eased = applyEasing(EasingMoveType::EaseOut, t);
		return 1.0 + (settings.camera.targetScale - 1.0) * eased;
	}

	Vec2 AvoidanceManager::getChapter4CameraCenter() const {
		const Vec2 screenCenter{ Global::windowWidth / 2.0, Global::windowHeight / 2.0 };
		if (activeChapter != 4 || !chapter4SightCameraZoomEnabled || chapter4SightCameraStartStep < 0) {
			return screenCenter;
		}

		const auto& settings = chapter4Settings;
		const int32 cameraAge = step - chapter4SightCameraStartStep;
		const double t = cameraAge / static_cast<double>(Max(settings.sight.shrinkStep, 1));
		const double eased = applyEasing(EasingMoveType::EaseIn, t);
		return screenCenter + (chapter4SightCameraTargetCenter - screenCenter) * eased;
	}
}
