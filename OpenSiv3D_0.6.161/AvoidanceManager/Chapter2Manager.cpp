#include "AvoidanceManager.h"

namespace {
	constexpr int32 chapter2MeasureCount = 3;
	constexpr int32 chapter2SatBarrageSpawnStep = 340;

	struct Chapter2MeasureWindow {
		int32 begin = 0;
		int32 end = 0;
		int32 judgeStep = 0;
		int32 topFrames = 0;
		int32 bottomFrames = 0;
		Array<int32> areaIndices;
		bool resultTop = true;
		bool completed = false;
	};

	Array<Chapter2MeasureWindow> chapter2MeasureWindows;
	bool chapter2SatBarrageCreated = false;
	bool chapter2SatResultBarrageCreated = false;

	struct Chapter2MeasureAreaSettings {
		// Measure guide frames become fully visible at spawn and once again after this delay.
		int32 guideSecondFlashDelayStep = 18;
		// Each guide flash fades out over this many steps.
		int32 guideFadeOutStep = 15;
		int32 fillAppearanceDuration = 30;
		int32 fillHoldAfterPulseStep = 8;
		int32 fillFadeOutStep = 18;
		double fillSpacing = 16.0;
		double fillScale = 1.1;
		double fillDepth = Iwanna::DrawDepth::Player + 20.0;
		Array<RectF> areas = {
			RectF{ 96.0, 112.0, 160.0, 96.0 },
			RectF{ 96.0, 208.0, 160.0, 96.0 },
			RectF{ 96.0, 304.0, 160.0, 96.0 },
			RectF{ 96.0, 400.0, 160.0, 96.0 },
			RectF{ 320.0, 112.0, 160.0, 128.0 },
			RectF{ 320.0, 240.0, 160.0, 128.0 },
			RectF{ 320.0, 368.0, 160.0, 128.0 },
		};
		Array<Array<int32>> measureAreaCandidateGroups = {
			Array<int32>{ 0, 1, 2, 3, 4, 5, 6 },
			Array<int32>{ 0, 1, 2, 3, 4, 5, 6 },
			Array<int32>{ 4, 5, 6 },
		};
		Array<ColorF> colors = {
			ColorF{ 0.35, 0.85, 1.0 },
			ColorF{ 1.0, 0.35, 0.85 },
		};
	};

	struct Chapter2SatBarrageWallSettings {
		double startX = 0.0;
		double targetX = 0.0;
	};

	struct Chapter2SatBarrageSettings {
		ColorF color = ColorF{ 0.0, 0.0, 0.0 };
		ColorF maskColor = ColorF{ 0.0, 0.0, 0.0, 0.38 };
		Array<Chapter2SatBarrageWallSettings> walls = {
			{ -32.0, 336.0 },
			{ Global::windowWidth + 32.0, 464.0 },
		};
		int32 moveTime = 30;
		int32 repeatIntervalStep = 15;
		double repeatInitialSpeed = 0.0;
		double repeatAcceleration = 0.5;
		double repeatMaxSpeed = 16.0;
		int32 repeatFadeInStep = 20;
		double frontCherryDepth = Iwanna::DrawDepth::Player + 10.0;
		double maskTopY = 0.0;
		double maskBottomY = Global::windowHeight;
		double topY = -16.0;
		double bottomY = 624.0;
		double cherrySpacingY = 16.0;
	};

	struct Chapter2SatResultBarrageSettings {
		int32 lineCount = 4;
		double leftX = 352.0;
		double rightX = 448.0;
		double topY = 48.0;
		double lineSpacingY = 20.0;
		double cherrySpacingX = 16.0;
		double gapHalfWidth = 16.0;
		Array<double> gapCentersX = {
			368.0,
			400.0,
			432.0,
		};
		double gravity = 0.8;
		double initialUpSpeed = 6.0;
		double maxFallSpeed = 30.0;
		int32 lineFallIntervalStep = 30;
		double scale = 1.0;
		int32 appearanceDuration = 20;
		int32 fallStartDelayAfterAppearance = 15;
		double mediumColumnSpacingY = 8.0;
		int32 mediumColumnFadeOutStep = 15;
		Array<ColorF> resultColors = {
			ColorF{ 0.25, 0.95, 0.45 },
			ColorF{ 1.0, 0.82, 0.20 },
			ColorF{ 1.0, 0.35, 0.25 },
		};
	};

	struct Chapter2SniperSightSettings {
		String textureName = U"sprCherryAllWhite";
		double radius = 58.0;
		int32 circleCherryCount = 50;
		double crossSpacing = 5.0;
		double crossExtend = 10.0;
		double outerScale = 0.5;
		double crossScale = 0.2;
		double centerScale = 0.55;
		double rotationSpeed = 0.04;
		int32 fadeOutStep = 60;
		int32 pulseDurationStep = 10;
		double pulseScale = 1.28;
		double screenShakeAmplitude = 6.0;
		int32 screenShakeDurationStep = 18;
		double screenShakeFrequency = 0.85;
		Array<int32> pulseSteps = {
			0,
			100,
			220,
			340,
		};
		ColorF outerColor = ColorF{ 0.0, 0.0, 0.0, 0.95 };
		ColorF crossColor = ColorF{ 0.0, 0.0, 0.0, 0.95 };
		ColorF centerColor = ColorF{ 1.0, 0.0, 0.0, 1.0 };
	};

	struct Chapter2SniperPenaltySettings {
		String textureName = U"sprCherryAllWhite";
		ColorF color = ColorF{ 1.0, 0.12, 0.12 };
		int32 count = 100;
		double minSpeed = 10.0;
		double maxSpeed = 15.0;
		double scale = 1.0;
		double depth = Iwanna::DrawDepth::Cherry + 9.0;
	};

	struct Chapter2MikuHandBarrageSettings {
		String textureName = U"sprCherryWhite";
		ColorF color = Palette::White;
		Vec2 centerOffsetFromMiku = Vec2{ 0.0, 0.0 };
		int32 lineCount = (Global::difficulty == Global::Difficulty::Medium) ? 4 : 3;
		double cherrySpacing = 64.0;
		double minRadius = 0.0;
		double maxRadius = 1000.0;
		double passiveScale = 0.5;
		double activeScale = 1.0;
		double passiveAlpha = 0.45;
		double activeAlpha = 1.0;
		int32 transitionStep = 20;
		double rotationSpeed = 0.01;
		double depth = Iwanna::DrawDepth::Cherry + 6.0;
	};

	struct Chapter2MikuHandBarrageController {
		double rate = 0.0;
		double startRate = 0.0;
		double targetRate = 0.0;
		int32 timer = 0;
		int32 transitionStep = 1;

		void request(bool isAttackState, int32 newTransitionStep) {
			startRate = rate;
			targetRate = isAttackState ? 1.0 : 0.0;
			timer = 0;
			transitionStep = Max(newTransitionStep, 1);
		}

		void update() {
			if (rate == targetRate) {
				return;
			}

			const double t = timer / static_cast<double>(transitionStep);
			const double eased = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);
			rate = startRate + (targetRate - startRate) * eased;

			if (transitionStep <= timer) {
				rate = targetRate;
			}

			++timer;
		}
	};

	struct Chapter2CherryRodController {
		int32 request = 0;
		int32 columnCount = 1;
		int32 staggerColumnCount = 1;
		int32 intervalStep = 0;
		bool waveActive = false;
		int32 waveStartRequest = 0;
		Iwanna::Chapter2CherryRodWaveSettings waveSettings;

		void move(int32 newStaggerColumnCount, int32 newIntervalStep) {
			staggerColumnCount = Max(newStaggerColumnCount, 1);
			intervalStep = Max(newIntervalStep, 0);
			++request;
		}

		void startWave(const Iwanna::Chapter2CherryRodWaveSettings& newWaveSettings) {
			waveSettings = newWaveSettings;
			waveSettings.periodStep = Max(waveSettings.periodStep, 1);
			waveSettings.columnStartIntervalStep = Max(waveSettings.columnStartIntervalStep, 0);
			waveSettings.fadeInStep = Max(waveSettings.fadeInStep, 1);
			waveActive = true;
			++waveStartRequest;
		}
	};

	Iwanna::Cherry::Behavior makeChapter2CherryRodBehavior(
		const Vec2& home,
		int32 columnIndex,
		double moveDirection,
		double moveDistance,
		int32 moveTime,
		int32 holdTime,
		const std::shared_ptr<Chapter2CherryRodController>& controller) {

		struct State {
			int32 seenRequest = 0;
			int32 seenWaveStartRequest = 0;
			int32 phase = 0;
			int32 timer = 0;
			int32 waitTimer = 0;
			int32 waitTime = 0;
			int32 waveTimer = 0;
		};

		auto state = std::make_shared<State>();

		return [
			home,
			columnIndex,
			moveDirection,
			moveDistance,
			moveTime,
			holdTime,
			controller,
			state
		](Iwanna::Cherry& self, int32) {
			const int32 moveDuration = Max(moveTime, 1);
			const int32 holdDuration = Max(holdTime, 0);

			if (state->seenRequest != controller->request) {
				state->seenRequest = controller->request;
				state->timer = 0;
				state->waitTimer = 0;

				const int32 staggerColumnCount = Max(controller->staggerColumnCount, 1);
				const int32 groupIndex = columnIndex % staggerColumnCount;
				state->waitTime = groupIndex * Max(controller->intervalStep, 0);
				state->phase = (state->waitTime == 0) ? 1 : -1;
			}

			if (state->seenWaveStartRequest != controller->waveStartRequest) {
				state->seenWaveStartRequest = controller->waveStartRequest;
				state->waveTimer = 0;
			}

			double waveOffset = 0.0;
			if (controller->waveActive) {
				const auto& wave = controller->waveSettings;
				const int32 columnFromRight = Max(controller->columnCount - 1 - columnIndex, 0);
				const int32 startWait = columnFromRight * wave.columnStartIntervalStep;
				const int32 activeWaveTimer = state->waveTimer - startWait;

				if (0 <= activeWaveTimer) {
					const double fade = Iwanna::applyEasing(
						Iwanna::EasingMoveType::EaseInOut,
						activeWaveTimer / static_cast<double>(wave.fadeInStep));
					const double waveT = activeWaveTimer / static_cast<double>(Max(wave.periodStep, 1));
					const double direction = (moveDirection > 0.0) ? wave.topDirection : wave.bottomDirection;
					const double phase = columnIndex * wave.columnPhaseStep * fade;
					waveOffset = direction * wave.amplitude * fade * Math::Sin(6.283185307179586 * waveT + phase);
				}

				++state->waveTimer;
			}

			if (state->phase == 0) {
				self.pos = home + Vec2{ 0, waveOffset };
				return;
			}

			if (state->phase == -1) {
				self.pos = home + Vec2{ 0, waveOffset };

				if (state->waitTimer < state->waitTime) {
					++state->waitTimer;
					return;
				}

				state->phase = 1;
				state->timer = 0;
			}

			double rate = 0.0;

			if (state->phase == 1) {
				rate = Iwanna::applyEasing(
					Iwanna::EasingMoveType::EaseOut,
					state->timer / static_cast<double>(moveDuration));

				++state->timer;
				if (state->timer > moveDuration) {
					state->phase = (holdDuration > 0) ? 2 : 3;
					state->timer = 0;
				}
			}
			else if (state->phase == 2) {
				rate = 1.0;

				++state->timer;
				if (state->timer > holdDuration) {
					state->phase = 3;
					state->timer = 0;
				}
			}
			else {
				rate = 1.0 - Iwanna::applyEasing(
					Iwanna::EasingMoveType::EaseIn,
					state->timer / static_cast<double>(moveDuration));

				++state->timer;
				if (state->timer > moveDuration) {
					state->phase = 0;
					state->timer = 0;
					rate = 0.0;
				}
			}

			self.pos = home + Vec2{ 0, moveDirection * moveDistance * rate + waveOffset };
		};
	}

	Array<std::shared_ptr<Chapter2CherryRodController>> chapter2CherryRodControllers;
	std::shared_ptr<Chapter2MikuHandBarrageController> chapter2MikuHandBarrageController;

	bool containsAnyAreaIndex(const Array<int32>& areaIndices, const Array<int32>& excludedAreaIndices) {
		for (const int32 areaIndex : areaIndices) {
			if (excludedAreaIndices.includes(areaIndex)) {
				return true;
			}
		}

		return false;
	}

	Array<int32> createRandomChapter2MeasureAreaPair(
		const Array<int32>& candidates,
		const Array<int32>& previousAreaIndices,
		const Array<int32>& nextCandidates = {}) {

		Array<Array<int32>> pairs;

		for (int32 i = 0; i < static_cast<int32>(candidates.size()); ++i) {
			for (int32 j = i + 1; j < static_cast<int32>(candidates.size()); ++j) {
				Array<int32> pair = {
					candidates[i],
					candidates[j],
				};

				if (containsAnyAreaIndex(pair, previousAreaIndices)) {
					continue;
				}

				if (!nextCandidates.isEmpty()) {
					int32 nextCandidateCount = 0;
					for (const int32 nextAreaIndex : nextCandidates) {
						if (!pair.includes(nextAreaIndex)) {
							++nextCandidateCount;
						}
					}

					if (nextCandidateCount < 2) {
						continue;
					}
				}

				pairs << pair;
			}
		}

		if (pairs.isEmpty()) {
			for (int32 i = 0; i < static_cast<int32>(candidates.size()); ++i) {
				for (int32 j = i + 1; j < static_cast<int32>(candidates.size()); ++j) {
					pairs << Array<int32>{ candidates[i], candidates[j] };
				}
			}
		}

		return pairs[Random(static_cast<int32>(pairs.size()) - 1)];
	}

	void resetChapter2MeasureWindows() {
		const Chapter2MeasureAreaSettings areaSettings;
		chapter2MeasureWindows = {
			// Local steps in chapter 2.
			Chapter2MeasureWindow{ 0, 100, 100 },
			Chapter2MeasureWindow{ 110, 220, 220 },
			Chapter2MeasureWindow{ 240, 340, 340 },
		};

		Array<int32> previousAreaIndices;
		for (int32 i = 0; i < static_cast<int32>(chapter2MeasureWindows.size()); ++i) {
			const Array<int32>& candidates = areaSettings.measureAreaCandidateGroups[
				i % areaSettings.measureAreaCandidateGroups.size()];
			const Array<int32> nextCandidates = (i + 1 < static_cast<int32>(chapter2MeasureWindows.size()))
				? areaSettings.measureAreaCandidateGroups[(i + 1) % areaSettings.measureAreaCandidateGroups.size()]
				: Array<int32>{};
			chapter2MeasureWindows[i].areaIndices = createRandomChapter2MeasureAreaPair(
				candidates,
				previousAreaIndices,
				nextCandidates);
			previousAreaIndices = chapter2MeasureWindows[i].areaIndices;
		}

		chapter2SatBarrageCreated = false;
		chapter2SatResultBarrageCreated = false;
	}

	Iwanna::Cherry::Behavior makeChapter2TimedGuideBehavior(
		int32 secondFlashDelayStep,
		int32 fadeOutStep) {

		return [secondFlashDelayStep, fadeOutStep](Iwanna::Cherry& self, int32 age) {
			const int32 fadeDuration = Max(fadeOutStep, 1);
			const int32 secondDelay = Max(secondFlashDelayStep, 0);
			const double firstT = age / static_cast<double>(fadeDuration);
			const double secondT = (age - secondDelay) / static_cast<double>(fadeDuration);
			double alpha = 0.0;

			if (0.0 <= firstT && firstT <= 1.0) {
				alpha = Max(alpha, 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, firstT));
			}

			if (0.0 <= secondT && secondT <= 1.0) {
				alpha = Max(alpha, 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, secondT));
			}

			self.alpha = alpha;
			self.setScale(0.65);
			self.canPlayerKill = false;

			if (secondDelay + fadeDuration < age) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter2MeasureFillBehavior(
		int32 fadeStartAge,
		int32 fadeOutStep) {

		return [fadeStartAge, fadeOutStep](Iwanna::Cherry& self, int32 age) {
			if (age < fadeStartAge) {
				self.alpha = 1.0;
				return;
			}

			const int32 fadeDuration = Max(fadeOutStep, 1);
			const double t = (age - fadeStartAge) / static_cast<double>(fadeDuration);
			self.alpha = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);

			if (1.0 <= t) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter2SatBarrageColumnBehavior(
		double startX,
		double targetX,
		int32 moveTime) {

		return [startX, targetX, moveTime](Iwanna::Cherry& self, int32 age) {
			const int32 duration = Max(moveTime, 1);
			const double t = age / static_cast<double>(duration);
			const double rate = Iwanna::applyEasing(
				Iwanna::EasingMoveType::EaseOut,
				t);
			self.pos.x = startX + (targetX - startX) * rate;

			if (1.0 <= t) {
				self.pos.x = targetX;
				return;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter2SatBarrageRepeatBehavior(
		double direction,
		double initialSpeed,
		double acceleration,
		double maxSpeed,
		int32 fadeInStep) {

		struct State {
			double speed = 0.0;
		};

		auto state = std::make_shared<State>();
		const double sign = (direction < 0.0) ? -1.0 : 1.0;

		return [
			sign,
			initialSpeed,
			acceleration,
			maxSpeed,
			fadeInStep,
			state
		](Iwanna::Cherry& self, int32 age) {
			self.alpha = Iwanna::applyEasing(
				Iwanna::EasingMoveType::EaseInOut,
				age / static_cast<double>(Max(fadeInStep, 1)));

			if (state->speed == 0.0) {
				state->speed = Max(initialSpeed, 0.0);
			}

			state->speed = Min(state->speed + Max(acceleration, 0.0), Max(maxSpeed, 0.0));
			self.pos.x += sign * state->speed;
		};
	}

	Iwanna::Cherry::Behavior makeChapter2SatResultFallBehavior(
		const Vec2& home,
		int32 rowIndex,
		int32 rowCount,
		double gravity,
		double initialUpSpeed,
		double maxFallSpeed,
		int32 rowFallIntervalStep,
		int32 startDelayStep) {

		struct State {
			bool started = false;
			double fallSpeed = 0.0;
			double fallOffset = 0.0;
		};

		auto state = std::make_shared<State>();
		const int32 rowFromBottom = Max(rowCount - 1 - rowIndex, 0);
		const int32 waitStep = Max(startDelayStep, 0) + rowFromBottom * Max(rowFallIntervalStep, 0);

		return [
			home,
			gravity,
			initialUpSpeed,
			maxFallSpeed,
			waitStep,
			state
		](Iwanna::Cherry& self, int32 age) {
			if (age < waitStep) {
				self.pos = home;
				return;
			}

			if (!state->started) {
				state->started = true;
				state->fallSpeed = -initialUpSpeed;
			}

			state->fallOffset += state->fallSpeed;
			state->fallSpeed = Min(state->fallSpeed + gravity, maxFallSpeed);
			self.pos = home + Vec2{ 0, state->fallOffset };
		};
	}

	Iwanna::Cherry::Behavior makeChapter2SatResultMediumSourceBehavior(
		const Vec2& home,
		int32 waitStep) {

		return [home, waitStep](Iwanna::Cherry& self, int32 age) {
			self.pos = home;
			self.canPlayerKill = false;

			if (Max(waitStep, 0) <= age) {
				self.isDelete = true;
			}
		};
	}

	Iwanna::Cherry::Behavior makeChapter2SatResultMediumColumnBehavior(
		int32 waitStep,
		int32 fadeOutStep,
		double scale) {

		return [waitStep, fadeOutStep, scale](Iwanna::Cherry& self, int32 age) {
			const int32 startStep = Max(waitStep, 0);
			if (age < startStep) {
				self.alpha = 0.0;
				self.setScale(scale);
				self.canPlayerKill = false;
				return;
			}

			const int32 fadeDuration = Max(fadeOutStep, 1);
			const double t = (age - startStep) / static_cast<double>(fadeDuration);
			const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, t);
			const double sizeRate = Max(1.0 - rate, 0.0);
			self.alpha = sizeRate;
			self.setScale(scale * sizeRate);
			self.canPlayerKill = (age == startStep);

			if (1.0 <= t) {
				self.isDelete = true;
			}
		};
	}
}

namespace Iwanna {
	void AvoidanceManager::createChapter2CherryRods(
		const Chapter2CherryRodSettings& settings) {

		const int32 columnCount = Max(settings.columnCount, 1);
		const double cherrySpacing = Max(settings.cherrySpacing, 1.0);
		const int32 rodCherryCount = Max(static_cast<int32>(std::ceil(settings.rodLength / cherrySpacing)) + 1, 1);
		auto controller = std::make_shared<Chapter2CherryRodController>();
		controller->columnCount = columnCount;
		chapter2CherryRodControllers << controller;

		const auto createRodCherry = [&](const Vec2& home, int32 columnIndex, double moveDirection) {
			createCherry(home, Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.color,
				.behavior = makeChapter2CherryRodBehavior(
					home,
					columnIndex,
					moveDirection,
					settings.moveDistance,
					settings.moveTime,
					settings.holdTime,
					controller),
				.canDeleteOutOfScreen = false,
				.canPlayerKill = settings.canPlayerKill,
				.depth = settings.depth,
				.scale = settings.scale,
			});
		};

		for (int32 column = 0; column < columnCount; ++column) {
			const double x = settings.startX + column * settings.columnSpacing;

			for (int32 row = 0; row < rodCherryCount; ++row) {
				const double offset = row * cherrySpacing;
				createRodCherry(Vec2{ x, settings.topY + offset }, column, 1.0);
				createRodCherry(Vec2{ x, settings.bottomY - offset }, column, -1.0);
			}
		}
	}

	void AvoidanceManager::requestChapter2CherryRodMove(int32 staggerColumnCount, int32 intervalStep) {
		for (const auto& controller : chapter2CherryRodControllers) {
			controller->move(staggerColumnCount, intervalStep);
		}
	}

	void AvoidanceManager::startChapter2CherryRodWave(const Chapter2CherryRodWaveSettings& settings) {
		for (const auto& controller : chapter2CherryRodControllers) {
			controller->startWave(settings);
		}
	}

	void createChapter2MeasureFrame(
		AvoidanceManager& manager,
		const RectF& rect,
		const ColorF& color,
		int32 secondFlashDelayStep,
		int32 fadeOutStep) {

		constexpr double spacing = 16.0;
		constexpr double scale = 0.65;
		const auto behavior = makeChapter2TimedGuideBehavior(secondFlashDelayStep, fadeOutStep);

		const auto createGuideCherry = [&](const Vec2& pos) {
			manager.createCherry(pos, Cherry::Settings{
				.textureName = U"sprCherryAllWhite",
				.color = color,
				.behavior = behavior,
				.canDeleteOutOfScreen = false,
				.canPlayerKill = false,
				.depth = DrawDepth::Cherry + 8.0,
				.scale = scale,
				.alpha = 0.45,
				.canPlayerKillBeforeFullAlpha = true,
				.manualCanPlayerKillControl = true,
			});
		};

		const int32 horizontalCount = Max(static_cast<int32>(rect.w / spacing), 1);
		const int32 verticalCount = Max(static_cast<int32>(rect.h / spacing), 1);

		for (int32 i = 0; i <= horizontalCount; ++i) {
			const double x = rect.x + rect.w * i / horizontalCount;
			createGuideCherry(Vec2{ x, rect.y });
			createGuideCherry(Vec2{ x, rect.y + rect.h });
		}

		for (int32 i = 1; i < verticalCount; ++i) {
			const double y = rect.y + rect.h * i / verticalCount;
			createGuideCherry(Vec2{ rect.x, y });
			createGuideCherry(Vec2{ rect.x + rect.w, y });
		}
	}

	void createChapter2SniperPenaltyBarrage(
		AvoidanceManager& manager,
		const Vec2& center) {

		const Chapter2SniperPenaltySettings settings;

		for (int32 i = 0; i < settings.count; ++i) {
			auto cherry = std::make_shared<Iwanna::Cherry>();
			cherry->reset(center, Iwanna::Cherry::Settings{
				.textureName = settings.textureName,
				.color = settings.color,
				.canDeleteOutOfScreen = true,
				.canPlayerKill = true,
				.depth = settings.depth,
				.scale = settings.scale,
			});
			cherry->speed = Random(settings.minSpeed, settings.maxSpeed);
			cherry->dir = Random(0.0, 360.0);
			manager.createCherry(cherry);
		}
	}

	void AvoidanceManager::createChapter2MeasureGuide(int32 index) {
		if (index < 0 || chapter2MeasureWindows.size() <= static_cast<size_t>(index)) {
			return;
		}

		const Chapter2MeasureAreaSettings areaSettings;
		const auto& window = chapter2MeasureWindows[index];

		for (int32 i = 0; i < static_cast<int32>(window.areaIndices.size()); ++i) {
			const int32 areaIndex = window.areaIndices[i];
			if (areaIndex < 0 || areaSettings.areas.size() <= static_cast<size_t>(areaIndex)) {
				continue;
			}

			createChapter2MeasureFrame(
				*this,
				areaSettings.areas[areaIndex],
				areaSettings.colors[i % areaSettings.colors.size()],
				areaSettings.guideSecondFlashDelayStep,
				areaSettings.guideFadeOutStep);
		}
	}

	void AvoidanceManager::createChapter2MeasureFill(int32 index) {
		if (index < 0 || chapter2MeasureWindows.size() <= static_cast<size_t>(index)) {
			return;
		}

		const Chapter2MeasureAreaSettings areaSettings;
		const auto& window = chapter2MeasureWindows[index];
		const int32 fadeStartAge = areaSettings.fillAppearanceDuration + areaSettings.fillHoldAfterPulseStep;
		const auto behavior = makeChapter2MeasureFillBehavior(
			fadeStartAge,
			areaSettings.fillFadeOutStep);

		for (int32 areaOrder = 0; areaOrder < static_cast<int32>(window.areaIndices.size()); ++areaOrder) {
			const int32 areaIndex = window.areaIndices[areaOrder];
			if (areaIndex < 0 || areaSettings.areas.size() <= static_cast<size_t>(areaIndex)) {
				continue;
			}

			const RectF& rect = areaSettings.areas[areaIndex];
			const ColorF color = areaSettings.colors[areaOrder % areaSettings.colors.size()];

			for (double y = rect.y; y <= rect.y + rect.h; y += areaSettings.fillSpacing) {
				for (double x = rect.x; x <= rect.x + rect.w; x += areaSettings.fillSpacing) {
					createCherry(Vec2{ x, y }, Cherry::Settings{
						.textureName = U"sprCherryAllWhite",
						.color = color,
						.behavior = behavior,
						.canDeleteOutOfScreen = false,
						.canPlayerKill = false,
						.depth = areaSettings.fillDepth,
						.scale = areaSettings.fillScale,
						.appearanceEffect = CherryEffect::ScaleIn,
						.appearanceDuration = areaSettings.fillAppearanceDuration,
					});
				}
			}
		}
	}

	Iwanna::Cherry::Behavior makeChapter2MikuHandBarrageBehavior(
		const std::shared_ptr<Iwanna::Miku>& miku,
		const std::shared_ptr<Chapter2MikuHandBarrageController>& controller,
		const Vec2& centerOffsetFromMiku,
		const Vec2& relativePos,
		double rotationSpeed,
		double passiveScale,
		double activeScale,
		double passiveAlpha,
		double activeAlpha) {

		return [
			miku,
			controller,
			centerOffsetFromMiku,
			relativePos,
			rotationSpeed,
			passiveScale,
			activeScale,
			passiveAlpha,
			activeAlpha
		](Iwanna::Cherry& self, int32 age) {
			if (!miku || !controller) {
				self.isDelete = true;
				return;
			}

			const double rotation = age * rotationSpeed;
			const double c = Math::Cos(rotation);
			const double s = Math::Sin(rotation);
			const Vec2 rotated{
				relativePos.x * c - relativePos.y * s,
				relativePos.x * s + relativePos.y * c,
			};
			self.pos = miku->pos + centerOffsetFromMiku + rotated;
			self.setScale(passiveScale + (activeScale - passiveScale) * controller->rate);
			self.alpha = passiveAlpha + (activeAlpha - passiveAlpha) * controller->rate;
			self.canPlayerKill = (0.999 <= controller->rate);
		};
	}

	void AvoidanceManager::updateChapter2Measurements(int32 localStep) {
		if (chapter2MeasureWindows.isEmpty()) {
			return;
		}

		const Chapter2MeasureAreaSettings areaSettings;
		const Vec2 playerPos = gameObjects.player->pos;

		for (auto& window : chapter2MeasureWindows) {
			if (!window.completed && window.judgeStep <= localStep) {
				bool isInsideAnyArea = false;

				if (0 < window.areaIndices.size()) {
					const int32 areaIndex = window.areaIndices[0];
					if (0 <= areaIndex
						&& static_cast<size_t>(areaIndex) < areaSettings.areas.size()
						&& areaSettings.areas[areaIndex].contains(playerPos)) {
						++window.topFrames;
						isInsideAnyArea = true;
					}
				}

				if (1 < window.areaIndices.size()) {
					const int32 areaIndex = window.areaIndices[1];
					if (0 <= areaIndex
						&& static_cast<size_t>(areaIndex) < areaSettings.areas.size()
						&& areaSettings.areas[areaIndex].contains(playerPos)) {
						++window.bottomFrames;
						isInsideAnyArea = true;
					}
				}

				if (!isInsideAnyArea) {
					createChapter2SniperPenaltyBarrage(*this, playerPos);
				}

				window.resultTop = (window.topFrames >= window.bottomFrames);
				window.completed = true;
			}
		}
	}

	void AvoidanceManager::createChapter2MikuHandBarrage() {
		if (!gameObjects.miku) {
			return;
		}

		const Chapter2MikuHandBarrageSettings settings;
		chapter2MikuHandBarrageController = std::make_shared<Chapter2MikuHandBarrageController>();
		chapter2MikuHandBarrageController->rate = 0.0;
		chapter2MikuHandBarrageController->targetRate = 0.0;
		chapter2MikuHandBarrageController->transitionStep = Max(settings.transitionStep, 1);

		const int32 lineCount = Max(settings.lineCount, 1);
		const double cherrySpacing = Max(settings.cherrySpacing, 1.0);
		const double minRadius = Max(settings.minRadius, 0.0);
		const double maxRadius = Max(settings.maxRadius, minRadius);
		const Vec2 center = gameObjects.miku->pos + settings.centerOffsetFromMiku;

		for (int32 line = 0; line < lineCount; ++line) {
			const double angle = 3.141592653589793 * line / lineCount;
			const Vec2 direction{ Math::Cos(angle), Math::Sin(angle) };

			for (double radius = minRadius; radius <= maxRadius; radius += cherrySpacing) {
				for (int32 side = -1; side <= 1; side += 2) {
					const double sign = static_cast<double>(side);
					const Vec2 relativePos = direction * radius * sign;
					createCherry(center + relativePos, Cherry::Settings{
						.textureName = settings.textureName,
						.color = settings.color,
						.behavior = makeChapter2MikuHandBarrageBehavior(
							gameObjects.miku,
							chapter2MikuHandBarrageController,
							settings.centerOffsetFromMiku,
							relativePos,
							settings.rotationSpeed,
							settings.passiveScale,
							settings.activeScale,
							settings.passiveAlpha,
							settings.activeAlpha),
						.canDeleteOutOfScreen = false,
						.canPlayerKill = false,
						.depth = settings.depth,
						.scale = settings.passiveScale,
						.alpha = settings.passiveAlpha,
						.manualCanPlayerKillControl = true,
					});
				}
			}
		}
	}

	void AvoidanceManager::requestChapter2MikuHandBarrageState(bool isAttackState) {
		const Chapter2MikuHandBarrageSettings settings;

		if (!chapter2MikuHandBarrageController) {
			return;
		}

		chapter2MikuHandBarrageController->request(isAttackState, settings.transitionStep);
	}

	void createChapter2SatBarrageWall(
		AvoidanceManager& manager,
		const Chapter2SatBarrageSettings& settings,
		double x,
		const Iwanna::Cherry::Behavior& behavior,
		bool canDeleteOutOfScreen) {

		for (double y = settings.topY; y <= settings.bottomY; y += settings.cherrySpacingY) {
			manager.createCherry(Vec2{ x, y }, Iwanna::Cherry::Settings{
				.textureName = U"sprCherryAllWhite",
				.color = settings.color,
				.behavior = behavior,
				.canDeleteOutOfScreen = canDeleteOutOfScreen,
				.canPlayerKill = true,
				.depth = settings.frontCherryDepth,
				.scale = 1.0,
			});
		}
	}

	void createChapter2SatBarrageRepeatWall(
		AvoidanceManager& manager,
		const Chapter2SatBarrageSettings& settings,
		double x,
		double direction) {

		for (double y = settings.topY; y <= settings.bottomY; y += settings.cherrySpacingY) {
			manager.createCherry(Vec2{ x, y }, Iwanna::Cherry::Settings{
				.textureName = U"sprCherryAllWhite",
				.color = settings.color,
				.behavior = makeChapter2SatBarrageRepeatBehavior(
					direction,
					settings.repeatInitialSpeed,
					settings.repeatAcceleration,
					settings.repeatMaxSpeed,
					settings.repeatFadeInStep),
				.canDeleteOutOfScreen = true,
				.canPlayerKill = true,
				.depth = settings.frontCherryDepth,
				.scale = 1.0,
				.alpha = 0.0,
				.canPlayerKillBeforeFullAlpha = true,
			});
		}
	}

	void AvoidanceManager::createChapter2SatBarrage() {
		if (chapter2MeasureWindows.size() < chapter2MeasureCount) {
			return;
		}

		const Chapter2SatBarrageSettings settings;

		for (const auto& wall : settings.walls) {
			createChapter2SatBarrageWall(
				*this,
				settings,
				wall.startX,
				makeChapter2SatBarrageColumnBehavior(
					wall.startX,
					wall.targetX,
					settings.moveTime),
				false);
		}
	}

	void AvoidanceManager::createChapter2SatResultBarrage() {
		if (chapter2MeasureWindows.size() < chapter2MeasureCount) {
			return;
		}

		const Chapter2SatResultBarrageSettings settings;

		Array<int32> resultIndices;
		for (int32 i = 0; i < chapter2MeasureCount; ++i) {
			const auto& window = chapter2MeasureWindows[i];
			const int32 base = window.resultTop ? 0 : 2;
			resultIndices << ((base + i) % 3);
		}
		resultIndices << ((resultIndices[0] + resultIndices[1] + resultIndices[2]) % 3);

		for (int32 line = 0; line < settings.lineCount; ++line) {
			const int32 resultIndex = resultIndices[line % resultIndices.size()];
			const ColorF color = settings.resultColors[resultIndex % settings.resultColors.size()];
			const double gapCenterX = settings.gapCentersX[resultIndex % settings.gapCentersX.size()];
			const double y = settings.topY + line * settings.lineSpacingY;
			const int32 rowFromBottom = Max(settings.lineCount - 1 - line, 0);
			const int32 rowStartDelayStep = settings.appearanceDuration
				+ settings.fallStartDelayAfterAppearance
				+ rowFromBottom * Max(settings.lineFallIntervalStep, 0);

			for (double x = settings.leftX; x <= settings.rightX; x += settings.cherrySpacingX) {
				if (Abs(x - gapCenterX) <= settings.gapHalfWidth) {
					continue;
				}

				const Vec2 home{ x, y };
				const bool isMedium = (Global::difficulty == Global::Difficulty::Medium);
				createCherry(home, Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = color,
					.behavior = isMedium
						? makeChapter2SatResultMediumSourceBehavior(home, rowStartDelayStep)
						: makeChapter2SatResultFallBehavior(
							home,
							line,
							settings.lineCount,
							settings.gravity,
							settings.initialUpSpeed,
							settings.maxFallSpeed,
							settings.lineFallIntervalStep,
							settings.appearanceDuration + settings.fallStartDelayAfterAppearance),
					.canDeleteOutOfScreen = true,
					.canPlayerKill = !isMedium,
					.depth = DrawDepth::Cherry + 7.0,
					.scale = settings.scale,
					.appearanceEffect = CherryEffect::ScaleIn,
					.appearanceDuration = settings.appearanceDuration,
					.manualCanPlayerKillControl = isMedium,
				});

				if (isMedium) {
					const double spacingY = Max(settings.mediumColumnSpacingY, 1.0);
					for (double columnY = y; columnY <= Global::windowHeight + spacingY; columnY += spacingY) {
						createCherry(Vec2{ x, columnY }, Cherry::Settings{
							.textureName = U"sprCherryAllWhite",
							.color = color,
							.behavior = makeChapter2SatResultMediumColumnBehavior(
								rowStartDelayStep,
								settings.mediumColumnFadeOutStep,
								settings.scale),
							.canDeleteOutOfScreen = false,
							.canPlayerKill = false,
							.depth = DrawDepth::Cherry + 7.0,
							.scale = settings.scale,
							.alpha = 0.0,
							.canPlayerKillBeforeFullAlpha = true,
							.manualCanPlayerKillControl = true,
						});
					}
				}
			}
		}
	}

	void AvoidanceManager::drawChapter2SatBarrageMasks() const {
		if (activeChapter != 2) {
			return;
		}

		const int32 localStep = step - Global::startStep_Chapter2;
		if (localStep < chapter2SatBarrageSpawnStep) {
			return;
		}

		const Chapter2SatBarrageSettings settings;
		const double t = (localStep - chapter2SatBarrageSpawnStep) / static_cast<double>(Max(settings.moveTime, 1));
		const double rate = Iwanna::applyEasing(Iwanna::EasingMoveType::EaseOut, t);
		const double topY = settings.maskTopY;
		const double height = Max(settings.maskBottomY - settings.maskTopY, 0.0);
		const double centerX = Global::windowWidth / 2.0;

		for (const auto& wall : settings.walls) {
			const double x = wall.startX + (wall.targetX - wall.startX) * rate;

			if (wall.targetX < centerX) {
				const double right = Clamp(x, 0.0, static_cast<double>(Global::windowWidth));
				if (0.0 < right) {
					RectF{ 0.0, topY, right, height }.draw(settings.maskColor);
				}
			}
			else {
				const double left = Clamp(x, 0.0, static_cast<double>(Global::windowWidth));
				if (left < Global::windowWidth) {
					RectF{ left, topY, Global::windowWidth - left, height }.draw(settings.maskColor);
				}
			}
		}
	}

	void AvoidanceManager::drawChapter2SniperSight() const {
		if (activeChapter != 2 || !gameObjects.player) {
			return;
		}

		const Chapter2SniperSightSettings settings;
		const Vec2 center = gameObjects.player->pos;
		const double lineRadius = settings.radius + settings.crossExtend;
		const ScopedRenderStates2D rs{ SamplerState::ClampNearest };
		const auto texture = TextureAsset(settings.textureName);
		const int32 localStep = step - Global::startStep_Chapter2;
		const double rotation = localStep * settings.rotationSpeed;
		const auto rotate = [rotation](const Vec2& v) {
			const double c = Math::Cos(rotation);
			const double s = Math::Sin(rotation);
			return Vec2{
				v.x * c - v.y * s,
				v.x * s + v.y * c,
			};
		};
		int32 latestPulseStep = -1;

		for (const int32 pulseStep : settings.pulseSteps) {
			if (pulseStep <= localStep) {
				latestPulseStep = Max(latestPulseStep, pulseStep);
			}
		}

		if (latestPulseStep < 0) {
			return;
		}

		const int32 pulseAge = localStep - latestPulseStep;
		const double sizeRate = (0 <= pulseAge && pulseAge <= settings.pulseDurationStep)
			? 1.0 + (settings.pulseScale - 1.0)
				* (1.0 - Iwanna::applyEasing(
					Iwanna::EasingMoveType::EaseOut,
					pulseAge / static_cast<double>(Max(settings.pulseDurationStep, 1))))
			: 1.0;
		const double fadeT = (localStep - latestPulseStep) / static_cast<double>(Max(settings.fadeOutStep, 1));
		const double alphaRate = 1.0 - Iwanna::applyEasing(Iwanna::EasingMoveType::EaseInOut, fadeT);
		ColorF outerColor = settings.outerColor;
		ColorF crossColor = settings.crossColor;
		ColorF centerColor = settings.centerColor;
		outerColor.a *= alphaRate;
		crossColor.a *= alphaRate;
		centerColor.a *= alphaRate;

		for (int32 i = 0; i < settings.circleCherryCount; ++i) {
			const double angle = 6.283185307179586 * i / Max(settings.circleCherryCount, 1) + rotation;
			const Vec2 pos = center + Vec2{ Math::Cos(angle), Math::Sin(angle) } * settings.radius * sizeRate;
			texture.scaled(settings.outerScale).drawAt(pos, outerColor);
		}

		for (double offset = -lineRadius; offset <= lineRadius; offset += settings.crossSpacing) {
			if (Abs(offset) <= settings.crossSpacing * 0.5) {
				continue;
			}

			texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ offset * sizeRate, 0.0 }), crossColor);
			texture.scaled(settings.crossScale).drawAt(center + rotate(Vec2{ 0.0, offset * sizeRate }), crossColor);
		}

		texture.scaled(settings.centerScale * sizeRate).drawAt(center, centerColor);
	}

	// step : 840 - 2139
	void AvoidanceManager::chapter2() {
		Timeline timeline(previousStep, step);
		const int32 localStep = step - Global::startStep_Chapter2;

		if (chapter2MeasureWindows.isEmpty()) {
			resetChapter2MeasureWindows();
		}

		if (chapter2MikuHandBarrageController) {
			chapter2MikuHandBarrageController->update();
		}

		timeline.at(Global::startStep_Chapter2, [&] {
			chapter2CherryRodControllers.clear();
			resetChapter2MeasureWindows();
			createChapter2CherryRods(Chapter2CherryRodSettings{
				.textureName = U"sprCherryAllWhite",
				.color = ColorF{ 0.95, 0.95, 1.0 },
				.startX = 16.0,
				.columnCount = 25,
				.columnSpacing = 32.0,
				.topY = -64.0,
				.bottomY = 670.0,
				.rodLength = 112,
				.cherrySpacing = 16.0,
				.scale = 1.00,
				.moveDistance = 44.0,
				.moveTime = 10,
				.holdTime = 0,
				.depth = DrawDepth::Cherry + 5.0,
				.canPlayerKill = true,
			});
			createChapter2MikuHandBarrage();
			requestChapter2MikuHandBarrageState(false); // scale 0.5 / 半透明 / 判定なし
		});

		updateChapter2Measurements(localStep);

		timeline.at(Global::startStep_Chapter2, [&] {
			createChapter2MeasureGuide(0);
			requestChapter2CherryRodMove(2, 15);
		});

		timeline.at(Global::startStep_Chapter2 + 15, [&] {
			requestChapter2MikuHandBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter2 + 85, [&] {
			if (Global::difficulty == Global::Difficulty::Easy) requestChapter2MikuHandBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter2 +114, [&] {
			createChapter2MeasureGuide(1);
			requestChapter2CherryRodMove(4, 10);
		});

		timeline.at(Global::startStep_Chapter2 + 112, [&] {
			if (Global::difficulty == Global::Difficulty::Easy) requestChapter2MikuHandBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter2 + 205, [&] {
			if (Global::difficulty == Global::Difficulty::Easy) requestChapter2MikuHandBarrageState(false);
		});

		timeline.at(Global::startStep_Chapter2 + 240, [&] {
			createChapter2MeasureGuide(2);
			requestChapter2CherryRodMove(2, 15);
		});

		timeline.at(Global::startStep_Chapter2 + 235, [&] {
			if (Global::difficulty == Global::Difficulty::Easy) requestChapter2MikuHandBarrageState(true);
		});

		timeline.at(Global::startStep_Chapter2 + 325, [&] {
			requestChapter2MikuHandBarrageState(false);
		});

		{
			const Chapter2SniperSightSettings sightSettings;
			for (const int32 pulseStep : sightSettings.pulseSteps) {
				if (pulseStep <= 0) {
					continue;
				}

				timeline.at(Global::startStep_Chapter2 + pulseStep, [&, sightSettings] {
					requestScreenShake(
						sightSettings.screenShakeAmplitude,
						sightSettings.screenShakeDurationStep,
						sightSettings.screenShakeFrequency);
				});
			}
		}

		{
			const Chapter2MeasureAreaSettings areaSettings;
			for (int32 i = 0; i < static_cast<int32>(chapter2MeasureWindows.size()); ++i) {
				const int32 fillStep = Global::startStep_Chapter2
					+ chapter2MeasureWindows[i].judgeStep
					- areaSettings.fillAppearanceDuration;
				timeline.at(fillStep, [&, i] {
					createChapter2MeasureFill(i);
				});
			}
		}

		timeline.at(Global::startStep_Chapter2 + chapter2SatBarrageSpawnStep, [&] {
			if (!chapter2SatBarrageCreated) {
				chapter2SatBarrageCreated = true;
				createChapter2SatBarrage();
			}
		});

		{
			const Chapter2SatBarrageSettings settings;
			const int32 repeatBeginStep = Global::startStep_Chapter2
				+ chapter2SatBarrageSpawnStep
				+ settings.moveTime;
			timeline.every(settings.repeatIntervalStep, repeatBeginStep, Global::startStep_Chapter3 - 1, [&](int32) {
				const double centerX = Global::windowWidth / 2.0;
				for (const auto& wall : settings.walls) {
					const double direction = (wall.targetX < centerX) ? -1.0 : 1.0;
					createChapter2SatBarrageRepeatWall(
						*this,
						settings,
						wall.targetX,
						direction);
				}
			});
		}

		timeline.at(Global::startStep_Chapter2 + 340, [&] {
			startChapter2CherryRodWave(Chapter2CherryRodWaveSettings{
				.amplitude = 24.0,
				.periodStep = 80,
				.columnPhaseStep = 0.45,
				.columnStartIntervalStep = 2,
				.fadeInStep = 20,
				.topDirection = 1.0,
				.bottomDirection = -1.0,
			});
		});

		timeline.at(Global::startStep_Chapter2 + chapter2SatBarrageSpawnStep, [&] {
			if (!chapter2SatResultBarrageCreated) {
				chapter2SatResultBarrageCreated = true;
				createChapter2SatResultBarrage();
			}
		});
	}
}
