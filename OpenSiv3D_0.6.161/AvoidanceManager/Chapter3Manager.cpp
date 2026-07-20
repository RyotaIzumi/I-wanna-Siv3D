#include "AvoidanceManager.h"

namespace {
	constexpr int32 lowerMachine = 0;
	constexpr int32 upperMachine = 1;
	constexpr int32 randomMachine = -1;
	constexpr int32 chapter3LoopFirstStep = 23;
	constexpr int32 chapter3LoopPressStep = 73;
	constexpr int32 chapter3LoopEndStep = 143;
	constexpr int32 chapter3LoopLength = chapter3LoopEndStep - chapter3LoopFirstStep + 1;
	constexpr int32 finalCorrectPressLeadTime = 50;
	constexpr double pressShakeDistance = 50.0;

	struct PressMachineLayout {
		int32 widthNum = 50;             // 横一列の林檎数
		int32 toothSegmentNum = 10;      // ギザギザを分割する数
		int32 toothDepthNum = 6;         // ギザギザの縦方向の長さ
		int32 offscreenFillScale = 5;    // 画面外を埋める林檎の倍率
		double cellSize = 16.0;          // 林檎同士の間隔
		double upperToothRootY = 100.0;  // 小さくすると上端へ寄る
		double lowerToothRootY = 516.0;  // 大きくすると下端へ寄る
		double pressDistance = 216.0;    // プレス時の移動距離
		int32 specialMovingSide = randomMachine; // 0: 下, 1: 上, -1: ランダム
	};

	// Chapter 3 のプレス機の形状はここでまとめて調整する。
	constexpr PressMachineLayout pressMachineLayout{};
	static_assert(pressMachineLayout.widthNum % pressMachineLayout.toothSegmentNum == 0);
	static_assert(pressMachineLayout.widthNum % pressMachineLayout.offscreenFillScale == 0);
	static_assert(randomMachine <= pressMachineLayout.specialMovingSide
		&& pressMachineLayout.specialMovingSide <= upperMachine);

	struct LargeCherryRowSettings {
		double scale = 12.0;
		int32 appearanceTime = 60;
		double flowSpeed = 1.0;
		int32 extraColumnsEachSide = 2;
	};

	constexpr LargeCherryRowSettings largeCherryRowSettings{};

	struct GuideColumnSettings {
		String textureName = U"sprCherryAllWhite";
		ColorF color = ColorF{ 0.3, 0.3, 0.3 };
		double targetAlpha = 0.35;
		double scale = 0.25;
		double spacing = 8.0;
		double baseX = 8.0;
		int32 rowDelay = 1;
		int32 fadeTime = 12;
	};

	const GuideColumnSettings guideColumnSettings{};

	struct HorizontalSweepSettings {
		String textureName = U"sprCherryWhite";
		ColorF color = Palette::White;
		int32 startStepOffset = 370;
		int32 count = 4;
		int32 rowDelay = 14;
		int32 fadeTime = 12;
		int32 moveTime = 140;
		double startX = 784.0;
		double targetX = 16.0;
		double topY = 240.0;
		double verticalSpacing = 50.0;
		double targetAlpha = 1.0;
		double disabledAlpha = 0.35;
		double scale = 1.0;
		bool canPlayerKill = true;
	};

	const HorizontalSweepSettings horizontalSweepSettings{};

	struct FinalPressSettings {
		double hiddenOffset = 100.0;
	};

	constexpr FinalPressSettings finalPressSettings{};

	struct CogHintSettings {
		int32 beginStepOffset = 370;
		int32 endStepOffset = 470;
		int32 interval = 50;
		int32 rotationTime = 45;
		double degrees = 90.0;
	};

	constexpr CogHintSettings cogHintSettings{};
	static_assert(cogHintSettings.beginStepOffset
		+ cogHintSettings.interval
		+ cogHintSettings.rotationTime
		<= cogHintSettings.endStepOffset);

	struct SpecialPressState {
		bool moveFinished = false;
	};

	struct PressMachinePattern {
		Array<int32> lowerTeeth;
		Array<int32> upperTeeth;
	};

	struct PressMachineController {
		Vec2 offset = Vec2{ 0,0 };
		Iwanna::EasingMove move;
		std::shared_ptr<SpecialPressState> specialPressState;
		int32 pressRequest = 0;
		int32 pushRequest = 0;
		int32 specialPressRequest = 0;
		int32 horizontalDirection = -1;
		double pushDistance = 0.0;
		bool movesOnSpecialPress = false;

		void update() {
			if (move.isActive()) {
				offset = move.update();
			}
		}

		void scrMoveEasing(int32 easingType, const Vec2& targetOffset, int32 stepCount) {
			move.start(Iwanna::fromGameMakerEasingType(easingType), offset, targetOffset, stepCount);
		}

		void requestPress() {
			++pressRequest;
		}

		void requestPush() {
			++pushRequest;
		}

		void requestSpecialPress() {
			specialPressState->moveFinished = false;
			++specialPressRequest;
		}

		void finishSpecialMove() {
			specialPressState->moveFinished = true;
		}

		bool isSpecialMoveFinished() const {
			return specialPressState->moveFinished;
		}

		bool canRemove() const {
			if (horizontalDirection < 0) {
				return offset.x < -1600.0;
			}

			return offset.x > 1600.0;
		}
	};

	struct LargeCherryRowController {
		Vec2 offset = Vec2{ 0,0 };
		Iwanna::EasingMove move;
		double horizontalSpeed = 0.0;

		void update() {
			if (move.isActive()) {
				offset.y = move.update().y;
			}
			offset.x += horizontalSpeed;
		}
	};

	struct CogController {
		Vec2 center = Vec2{ 400,304 };
		double angle = 0.0;
		double cosAngle = 1.0;
		double sinAngle = 0.0;
		Iwanna::EasingValue rotation;

		void update() {
			if (rotation.isActive()) {
				angle = rotation.update();
			}

			// 同じ歯車に属する全 Cherry で共通の値なので、1 フレームに一度だけ計算する。
			const double rad = Math::ToRadians(angle);
			cosAngle = Math::Cos(rad);
			sinAngle = Math::Sin(rad);
		}

		void rotateRight(double degrees = 90.0, int32 stepCount = 50, int32 easingType = 2) {
			rotation.start(Iwanna::fromGameMakerEasingType(easingType), angle, angle + degrees, stepCount);
		}

		void rotateLeft(double degrees = 90.0, int32 stepCount = 50, int32 easingType = 2) {
			rotation.start(Iwanna::fromGameMakerEasingType(easingType), angle, angle - degrees, stepCount);
		}
	};

	double bezierEase(double t, double p1, double p2) {
		t = Clamp(t, 0.0, 1.0);
		const double inv = 1.0 - t;
		return 3.0 * inv * inv * t * p1 + 3.0 * inv * t * t * p2 + t * t * t;
	}

	double easeInOut(double t) {
		t = Clamp(t, 0.0, 1.0);
		return t * t * (3.0 - 2.0 * t);
	}

	Iwanna::Cherry::Behavior makePressMachineBehavior(
		const Vec2& localHome,
		int32 upDownPattern,
		const std::shared_ptr<PressMachineController>& controller,
		double pressDistance) {

		struct State {
			Vec2 localHome;
			Vec2 pressHome;
			Vec2 phaseStart;
			Vec2 phaseEnd;
			int32 upDown = lowerMachine;
			int32 phase = 0;
			int32 timer = 0;
			int32 seenPressRequest = 0;
			int32 seenPushRequest = 0;
			int32 seenSpecialPressRequest = 0;
			int32 pressCount = 0;
		};

		auto state = std::make_shared<State>();
		state->localHome = localHome;
		state->upDown = upDownPattern;

		return [state, controller, pressDistance](Iwanna::Cherry& self, int32) {
			constexpr int32 shakeTime = 23;
			constexpr int32 moveTime = 20;
			constexpr int32 laughMoveTime = 11; // わっはっはのときの一回の移動時間
			constexpr double shakeWidthX = 1.0;

			const Vec2 currentHome = state->localHome + controller->offset;
			const double sign = (state->upDown == lowerMachine) ? 1.0 : -1.0;

			if ((controller->horizontalDirection < 0 && currentHome.x < -100.0)
				|| (controller->horizontalDirection > 0 && currentHome.x > Global::windowWidth + 100.0)) {
				self.isDelete = true;
				return;
			}

			if (state->phase == 0) {
				self.pos = currentHome;

				if (state->seenSpecialPressRequest != controller->specialPressRequest) {
					state->seenSpecialPressRequest = controller->specialPressRequest;
					state->pressHome = currentHome;
					state->timer = 0;
					state->pressCount = 0;
					state->phase = 4;
				}

				else if (state->seenPushRequest != controller->pushRequest) {
					state->seenPushRequest = controller->pushRequest;
					state->pressHome = currentHome;
					state->phaseStart = currentHome;
					state->phaseEnd = currentHome + Vec2{ 0, -sign * controller->pushDistance };
					state->timer = 0;
					state->phase = 2;
				}

				else if (state->seenPressRequest != controller->pressRequest) {
					state->seenPressRequest = controller->pressRequest;
					state->pressHome = currentHome;
					state->timer = 0;

					state->phase = 1;
				}

				return;
			}

			switch (state->phase) {
			case 1: {// shake
				const double t = state->timer / static_cast<double>(shakeTime);
				const double e = bezierEase(t, 0.5, 0.99);
				self.pos.x = state->pressHome.x - shakeWidthX * Math::Sin(Math::ToRadians(720.0 * e));
				self.pos.y = state->pressHome.y + sign * pressShakeDistance * e;

				++state->timer;
				if (state->timer > shakeTime) {
					state->phaseStart = self.pos;
					state->phaseEnd = self.pos + Vec2{ 0, -sign * pressDistance };
					state->timer = 0;
					state->phase = 2;
				}
				break;
			}

			case 2: {// move
				const double t = state->timer / static_cast<double>(moveTime);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * bezierEase(t, 0.5, 0.99);

				++state->timer;
				if (state->timer > moveTime) {
					state->phaseStart = self.pos;
					state->phaseEnd = state->pressHome;
					state->timer = 0;
					state->phase = 3;
				}
				break;
			}

			case 3: {// move back
				const double t = state->timer / static_cast<double>(moveTime);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * bezierEase(t, 0.01, 0.5);

				++state->timer;
				if (state->timer > moveTime) {
					state->timer = 0;
					state->phase = 0;
				}
				break;
			}

			case 4: {// special phase shake
				const double t = state->timer / static_cast<double>(shakeTime);
				const double e = bezierEase(t, 0.5, 0.99);
				self.pos.x = state->pressHome.x - shakeWidthX * Math::Sin(Math::ToRadians(720.0 * e));
				self.pos.y = state->pressHome.y + sign * pressShakeDistance * e;

				++state->timer;
				if (state->timer > shakeTime) {
					state->timer = 0;

					if (controller->movesOnSpecialPress) {
						state->phaseStart = self.pos;
						state->phaseEnd = self.pos + Vec2{ 0, -sign * (pressDistance * 2 / 3) };
						state->phase = 5;
					}
					else {
						state->phaseStart = self.pos;
						state->phase = 7;
					}
				}
				break;
			}
			case 5: {// special phase move
				const double t = state->timer / static_cast<double>(laughMoveTime);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * bezierEase(t, 0.5, 0.9);

				++state->timer;
				if (state->timer > laughMoveTime) {
					state->timer = 0;
					state->pressCount++;

					if (state->pressCount >= 3) {
						state->phaseStart = self.pos;
						state->phaseEnd = state->pressHome + Vec2(0, ((state->upDown == lowerMachine) ? 1.0 : -1.0) * 100);
						controller->finishSpecialMove();
						state->phase = 6;
					}
					else
					{
						state->phaseStart = self.pos;
						state->phaseEnd = self.pos + Vec2{ 0, -sign * (pressDistance * 2 / 3) };
						state->phase = 5;
					}
				}
				break;
			}

			case 6: {// special phase move back
				const double t = state->timer / static_cast<double>(moveTime + 20);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * bezierEase(t, 0.01, 0.4);

				++state->timer;
				if (state->timer > moveTime + 20) {
					state->timer = 0;
					state->phase = 8;
				}
				break;
			}

			case 7: {// special phase wait
				self.pos = state->phaseStart;

				if (controller->isSpecialMoveFinished()) {
					state->phaseStart = self.pos;
					state->phaseEnd = state->pressHome + Vec2(0, ((state->upDown == lowerMachine) ? 1.0 : -1.0) * 100);
					state->timer = 0;
					state->phase = 6;
				}
				break;
			}

			}
		};
	}

	Iwanna::Cherry::Behavior makeCogBehavior(
		const Vec2& localOffset,
		const std::shared_ptr<CogController>& controller) {

		return [localOffset, controller](Iwanna::Cherry& self, int32) {
			const Vec2 rotated{
				localOffset.x * controller->cosAngle - localOffset.y * controller->sinAngle,
				localOffset.x * controller->sinAngle + localOffset.y * controller->cosAngle
			};

			self.pos = controller->center + rotated;
		};
	}

	Iwanna::Cherry::Behavior makeLargeCherryRowBehavior(
		const Vec2& localPos,
		const std::shared_ptr<LargeCherryRowController>& controller) {

		return [localPos, controller](Iwanna::Cherry& self, int32) {
			self.pos = localPos + controller->offset;
		};
	}

	Array<std::shared_ptr<PressMachineController>> pressMachines;
	Array<std::shared_ptr<CogController>> cogControllers;
	Array<std::shared_ptr<LargeCherryRowController>> largeCherryRowControllers;
	int32 selectedSpecialMovingSide = lowerMachine;
	int32 selectedMarkerPress = 1;
	int32 selectedCorrectPress = 1;
	PressMachinePattern correctPressPattern;
	Array<int32> cogHintDirections;
	bool horizontalSweepStartsFromRight = true;
	bool horizontalSweepStartsFromTop = true;
	bool horizontalSweepDisabled = false;
}

namespace Iwanna {
	// step : 1320 - 1799
	void AvoidanceManager::chapter3() {
		Timeline timeline(previousStep, step);

		timeline.at(Global::startStep_Chapter3, [&] {
			pressMachines.clear();
			cogControllers.clear();
			largeCherryRowControllers.clear();
			selectedSpecialMovingSide = (pressMachineLayout.specialMovingSide == randomMachine)
				? Random(lowerMachine, upperMachine)
				: pressMachineLayout.specialMovingSide;
			selectedMarkerPress = Random(1, 2);
			selectedCorrectPress = Random(1, 3);
			horizontalSweepStartsFromRight = (Random(0, 1) == 1);
			horizontalSweepStartsFromTop = (Random(0, 1) == 1);
			horizontalSweepDisabled = false;
			correctPressPattern = PressMachinePattern{};
			cogHintDirections.clear();

			switch (selectedCorrectPress) {
			case 1:
				cogHintDirections = { -1, -1 };
				break;
			case 2:
				cogHintDirections = { -1 };
				break;
			case 3:
				if (Random(0, 1) == 0) {
					cogHintDirections = { -1, 1 };
				}
				break;
			default:
				break;
			}
		});

		for (auto& machine : pressMachines) {
			machine->update();
		}
		for (auto& cog : cogControllers) {
			cog->update();
		}
		for (auto& row : largeCherryRowControllers) {
			row->update();
		}
		pressMachines.remove_if([](const auto& machine) {
			return machine->canRemove();
		});

		const auto createCog = [&](const Vec2& center, int32 teethNum) {
			constexpr int32 cirNum = 7;
			constexpr int32 outCirNum = 5;
			constexpr int32 minCirNum = 15;
			constexpr int32 addCirNum = 5;
			constexpr double startRadius = 40.0;
			constexpr double addRadius = 10.0;

			auto controller = std::make_shared<CogController>();
			controller->center = center;
			cogControllers << controller;

			const auto createCogCherry = [&](const Vec2& localOffset) {
				createCherry(center + localOffset, Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = ColorF(1.0, 1.00, 1.00),
					.behavior = makeCogBehavior(localOffset, controller),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = false,
					.depth = DrawDepth::Cherry - 1.0,
					.appearanceEffect = CherryEffect::ScaleIn,
					.appearanceDuration = 20,
				});
			};

			double radius = startRadius;

			for (int32 j = 0; j < cirNum; ++j) {
				const int32 bulletNum = minCirNum + j * addCirNum;

				for (int32 i = 0; i < bulletNum; ++i) {
					const double angle = -90.0 + 360.0 / bulletNum * i;
					createCogCherry(Vec2{
						radius * Math::Cos(Math::ToRadians(angle)),
						radius * Math::Sin(Math::ToRadians(angle))
					});
				}

				radius += addRadius;
			}

			const int32 outerBulletNum = minCirNum + addCirNum * cirNum;
			const double cogDir = 360.0 / (teethNum * 2.0);

			for (int32 j = 0; j < outCirNum; ++j) {
				for (int32 i = 0; i < outerBulletNum; ++i) {
					const double angle = 360.0 / outerBulletNum * i;
					const double mod = std::fmod(angle, cogDir * 2.0);

					if (cogDir <= mod && mod < cogDir * 2.0) {
						createCogCherry(Vec2{
							radius * Math::Cos(Math::ToRadians(angle)),
							radius * Math::Sin(Math::ToRadians(angle))
						});
					}
				}

				radius += addRadius;
			}
		};

		const auto createPressMachine = [&](
			double baseX,
			bool createWhiteMarker,
			int32 pressNumber,
			const PressMachinePattern* savedPattern) {
			const auto& layout = pressMachineLayout;
			const int32 toothWidth = layout.widthNum / layout.toothSegmentNum;
			auto specialPressState = std::make_shared<SpecialPressState>();
			const int32 specialMovingSide = selectedSpecialMovingSide;
			PressMachinePattern pattern;

			if (savedPattern) {
				pattern = *savedPattern;
			}
			else {
				pattern.lowerTeeth.resize(layout.toothSegmentNum);
				pattern.upperTeeth.resize(layout.toothSegmentNum);

				for (int32 i = 0; i < layout.toothSegmentNum; ++i) {
					pattern.lowerTeeth[i] = Random(0, 1);
					pattern.upperTeeth[i] = pattern.lowerTeeth[i] + 1;
				}

				const int32 safeIndex = Random(3, 5);
				pattern.lowerTeeth[safeIndex] = 0;
				pattern.upperTeeth[safeIndex] = 0;
			}

			if (pressNumber == selectedCorrectPress) {
				correctPressPattern = pattern;
			}

			const auto createMachineCherry = [&](const Vec2& localPos, int32 upDownPattern, const ColorF& color, double depth, double scale, const std::shared_ptr<PressMachineController>& controller) {
				createCherry(localPos + controller->offset, Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = color,
					.behavior = makePressMachineBehavior(localPos, upDownPattern, controller, layout.pressDistance),
					.canDeleteOutOfScreen = false,
					.canPlayerKill = true,
					.depth = depth,
					.scale = scale,
				});
			};

			const auto createMachineSide = [&](int32 upDownPattern, double sideBaseX, double toothRootY, int32 horizontalDirection) {
				auto controller = std::make_shared<PressMachineController>();
				controller->specialPressState = specialPressState;
				controller->movesOnSpecialPress = (upDownPattern == specialMovingSide);
				controller->horizontalDirection = horizontalDirection;
				pressMachines << controller;
				Array<Point> machineCells;
				HashTable<Point, bool> occupiedCells;

				const auto addMachineCell = [&](const Point& cell) {
					if (!occupiedCells.contains(cell)) {
						machineCells << cell;
						occupiedCells[cell] = true;
					}
				};

				// 歯の反対側は、プレス後にも画面端まで覆う位置まで生成する。
				const double bodyEndY = (upDownPattern == lowerMachine)
					? (Global::windowHeight + layout.pressDistance + layout.cellSize) + Global::windowHeight / 2
					: (0.0 - layout.pressDistance - layout.cellSize) - Global::windowHeight / 2;
				const int32 bodyDirection = (upDownPattern == lowerMachine) ? 1 : -1;
				double firstOffscreenY = 0.0;
				bool foundOffscreenRow = false;

				for (int32 row = 1; ; ++row) {
					const double y = toothRootY + bodyDirection * row * layout.cellSize;

					if ((upDownPattern == lowerMachine) ? (y > bodyEndY) : (y < bodyEndY)) {
						break;
					}

					if (0.0 <= y && y <= Global::windowHeight) {
						for (int32 i = 0; i < layout.widthNum; ++i) {
							addMachineCell(Point{ i, bodyDirection * row });
						}
					}
					else if (!foundOffscreenRow) {
						firstOffscreenY = y;
						foundOffscreenRow = true;
					}
				}

				const int32 toothDirection = -bodyDirection;
				for (int32 i = 0; i < layout.toothDepthNum; ++i) {
					for (int32 k = 0; k < layout.toothSegmentNum; ++k) {
						const bool shouldCreate = (upDownPattern == lowerMachine)
							? (pattern.lowerTeeth[k] == 1)
							: (pattern.upperTeeth[k] == 1);

						if (!shouldCreate) {
							continue;
						}

						for (int32 j = 0; j < toothWidth; ++j) {
							addMachineCell(Point{
								toothWidth * k + j,
								toothDirection * i
							});
						}
					}
				}

				const ColorF bodyColor{ 0.38, 0.40, 0.42 };
				const ColorF warningYellow{ 1.0, 0.72, 0.05 };
				const ColorF warningBlack{ 0.06, 0.06, 0.07 };
				const ColorF largeScaleColor{ 0.06, 0.06, 0.07 };
				const double largeScale = static_cast<double>(layout.offscreenFillScale);
				const double largeCellSize = layout.cellSize * largeScale;
				const double largeHalfSize = largeCellSize / 2.0;
				const int32 largeWidthNum = layout.widthNum / layout.offscreenFillScale;
				double largeY = firstOffscreenY
					+ bodyDirection * (layout.offscreenFillScale - 1) * layout.cellSize / 2.0;
				Array<Vec2> largeCherryPositions;
				Array<int32> markerCandidateIndices;

				for (;;) {
					for (int32 i = 0; i < largeWidthNum; ++i) {
						const Vec2 localPos{
							sideBaseX + (i * layout.offscreenFillScale + (layout.offscreenFillScale - 1) / 2.0) * layout.cellSize,
							largeY
						};
						const double pressedY = localPos.y
							- bodyDirection * (layout.pressDistance - pressShakeDistance);
						const int32 positionIndex = static_cast<int32>(largeCherryPositions.size());
						largeCherryPositions << localPos;

						if (0.0 <= pressedY && pressedY <= Global::windowHeight) {
							markerCandidateIndices << positionIndex;
						}
					}

					const bool reachedBodyEnd = (upDownPattern == lowerMachine)
						? (largeY + largeHalfSize >= bodyEndY)
						: (largeY - largeHalfSize <= bodyEndY);
					if (reachedBodyEnd) {
						break;
					}

					largeY += bodyDirection * largeCellSize;
				}

				int32 markerIndex = -1;
				if (createWhiteMarker && !controller->movesOnSpecialPress && !markerCandidateIndices.isEmpty()) {
					markerIndex = markerCandidateIndices[Random(static_cast<int32>(markerCandidateIndices.size() - 1))];
				}

				for (int32 i = 0; i < static_cast<int32>(largeCherryPositions.size()); ++i) {
					const bool isMarker = (i == markerIndex);
					createMachineCherry(
						largeCherryPositions[i],
						upDownPattern,
						isMarker ? ColorF{ 1.0 } : largeScaleColor,
						isMarker ? DrawDepth::Cherry : DrawDepth::Cherry - 0.1,
						largeScale,
						controller);
				}

				for (const auto& cell : machineCells) {
					const bool isOutline = !occupiedCells.contains(cell + Point{ -1,0 })
						|| !occupiedCells.contains(cell + Point{ 1,0 })
						|| !occupiedCells.contains(cell + Point{ 0,toothDirection });
					const int32 stripeIndex = ((cell.x + cell.y + 1024) / 2) % 2;
					const ColorF color = isOutline
						? ((stripeIndex == 0) ? warningYellow : warningBlack)
						: bodyColor;
					const double depth = isOutline
						? DrawDepth::Cherry + 1.0
						: DrawDepth::Cherry;
					const Vec2 localPos{
						sideBaseX + cell.x * layout.cellSize,
						toothRootY + cell.y * layout.cellSize
					};

					createMachineCherry(localPos, upDownPattern, color, depth, 1.0, controller);
				}

				return controller;
			};

			Array<std::shared_ptr<PressMachineController>> createdControllers;
			createdControllers << createMachineSide(lowerMachine, baseX, layout.lowerToothRootY, -1);
			createdControllers << createMachineSide(upperMachine, -baseX + layout.cellSize, layout.upperToothRootY, 1);
			return createdControllers;
		};

		const auto createLargeCherryRows = [&] {
			const auto& settings = largeCherryRowSettings;
			const double cherrySize = pressMachineLayout.cellSize * settings.scale;
			const double halfSize = cherrySize / 2.0 - 15.0; // Adjust this value as needed
			const int32 visibleWidthNum = static_cast<int32>(std::ceil(Global::windowWidth / cherrySize));
			const int32 widthNum = visibleWidthNum + settings.extraColumnsEachSide * 2;
			const double startX = halfSize - settings.extraColumnsEachSide * cherrySize;

			for (const int32 direction : { -1, 1 }) {
				auto controller = std::make_shared<LargeCherryRowController>();
				controller->offset = Vec2{ 0, direction * cherrySize };
				controller->horizontalSpeed = -direction * settings.flowSpeed;
				controller->move.start(
					EasingMoveType::EaseOut,
					controller->offset,
					Vec2{ 0,0 },
					settings.appearanceTime);
				largeCherryRowControllers << controller;

				const double targetY = (direction < 0)
					? halfSize
					: Global::windowHeight - halfSize;

				for (int32 i = 0; i < widthNum; ++i) {
					const Vec2 localPos{ startX + i * cherrySize, targetY };
					createCherry(localPos + controller->offset, Cherry::Settings{
						.textureName = U"sprCherryWhite",
						.color = ColorF{0.9},
						.behavior = makeLargeCherryRowBehavior(localPos, controller),
						.canDeleteOutOfScreen = false,
						.canPlayerKill = true,
						.depth = DrawDepth::Cherry + 10,
						.scale = settings.scale,
					});
				}
			}
		};

		const auto createGuideColumns = [&] {
			const auto& settings = guideColumnSettings;
			const int32 teethWidth = pressMachineLayout.widthNum / pressMachineLayout.toothSegmentNum;
			const double segmentWidth = teethWidth * pressMachineLayout.cellSize;
			const int32 heightNum = static_cast<int32>(std::ceil(Global::windowHeight / settings.spacing));

			// 左右端を除く、ギザギザ区画同士の境界に縦列を作る。
			for (int32 boundary = 1; boundary < pressMachineLayout.toothSegmentNum; ++boundary) {
				const double x = settings.baseX + boundary * segmentWidth;

				for (int32 row = 0; row < heightNum; ++row) {
					const double y = settings.spacing / 2.0 + row * settings.spacing;
					const int32 fadeDelay = row * settings.rowDelay;
					const int32 fadeTime = Max(settings.fadeTime, 1);
					const double targetAlpha = settings.targetAlpha;
					createCherry(Vec2{ x,y }, Cherry::Settings{
						.textureName = settings.textureName,
						.color = settings.color,
						.behavior = [fadeDelay, fadeTime, targetAlpha](Cherry& self, int32 age) {
							const double t = (age - fadeDelay) / static_cast<double>(fadeTime);
							self.alpha = targetAlpha * applyEasing(EasingMoveType::EaseInOut, t);
						},
						.canDeleteOutOfScreen = false,
						.canPlayerKill = false,
						.depth = DrawDepth::Cherry + 1.0,
						.scale = settings.scale,
						.alpha = 0.0,
					});
				}
			}
		};

		const auto createHorizontalSweep = [&] {
			const auto& settings = horizontalSweepSettings;
			const int32 fadeTime = Max(settings.fadeTime, 1);
			const int32 moveTime = Max(settings.moveTime, 1);

			for (int32 row = 0; row < settings.count; ++row) {
				const int32 moveDelay = fadeTime + row * settings.rowDelay;
				const int32 verticalIndex = horizontalSweepStartsFromTop
					? row
					: settings.count - 1 - row;
				const double y = settings.topY + verticalIndex * settings.verticalSpacing;
				const double startX = horizontalSweepStartsFromRight
					? settings.startX
					: settings.targetX;
				const double targetX = horizontalSweepStartsFromRight
					? settings.targetX
					: settings.startX;
				const double targetAlpha = settings.targetAlpha;
				const double disabledAlpha = settings.disabledAlpha;
				createCherry(Vec2{ startX,y }, Cherry::Settings{
					.textureName = settings.textureName,
					.color = settings.color,
					.behavior = [moveDelay, fadeTime, moveTime, startX, targetX, targetAlpha, disabledAlpha](Cherry& self, int32 age) {
						const auto applyAttackState = [&](double normalAlpha) {
							self.alpha = horizontalSweepDisabled ? disabledAlpha : normalAlpha;
							if (horizontalSweepDisabled) {
								self.canPlayerKill = false;
							}
						};

						if (age < fadeTime) {
							const double fadeT = age / static_cast<double>(fadeTime);
							applyAttackState(targetAlpha * applyEasing(EasingMoveType::EaseInOut, fadeT));
							self.pos.x = startX;
							return;
						}

						applyAttackState(targetAlpha);
						if (age < moveDelay) {
							self.pos.x = startX;
							return;
						}

						const double moveT = (age - moveDelay) / static_cast<double>(moveTime);
						const double eased = applyEasing(EasingMoveType::EaseInOut, moveT);
						self.pos.x = startX + (targetX - startX) * eased;
					},
					.canDeleteOutOfScreen = false,
					.canPlayerKill = settings.canPlayerKill,
					.depth = DrawDepth::Cherry + 12.0,
					.scale = settings.scale,
					.alpha = 0.0,
				});
			}
		};

		timeline.at(Global::startStep_Chapter3 + 1, [&] {
			createCog(Vec2{ 400,304 }, 8);
			createCog(Vec2{ 100,304 }, 8);
			createCog(Vec2{ 700,304 }, 8);
			createPressMachine(8, false, 0, nullptr);
			createPressMachine(808.0, selectedMarkerPress == 1, 1, nullptr);
		});

		timeline.every(
			chapter3LoopLength,
			Global::startStep_Chapter3 + chapter3LoopFirstStep,
			Global::startStep_Chapter3 + chapter3LoopFirstStep + chapter3LoopLength * 2 - 1,
			[&](int32 localStep) {
				for (auto& machine : pressMachines) {
					machine->scrMoveEasing(2, machine->offset + Vec2{ 800.0 * machine->horizontalDirection,0 }, 50);
				}
				for (auto& cog : cogControllers) {
					cog->rotateRight(90.0, 50, 2);
				}

				const int32 incomingPressNumber = localStep / chapter3LoopLength + 2;
				createPressMachine(
					808.0,
					incomingPressNumber == selectedMarkerPress,
					incomingPressNumber,
					nullptr);
			});

		timeline.every(
			chapter3LoopLength,
			Global::startStep_Chapter3 + chapter3LoopPressStep,
			Global::startStep_Chapter3 + chapter3LoopPressStep * 3 + 1,
			[&](int32) {
				for (auto& machine : pressMachines) {
					machine->requestPress();
				}
			});

		// 冗談~
		timeline.at(Global::startStep_Chapter3 + chapter3LoopFirstStep + chapter3LoopLength * 2, [&] {
			for (auto& machine : pressMachines) {
				machine->scrMoveEasing(2, machine->offset + Vec2{ 800.0 * machine->horizontalDirection,0 }, 30);
			}
			for (auto& cog : cogControllers) {
				cog->rotateRight(90.0, 25, 2);
			}
		});

		// ばかりね
		timeline.at(Global::startStep_Chapter3 + chapter3LoopFirstStep + chapter3LoopLength * 2 + 30, [&] {
			for (auto& machine : pressMachines) {
				machine->requestSpecialPress();
			}
		});

		timeline.at(Global::startStep_Chapter3 + 370, [&] {
			createLargeCherryRows();
			createGuideColumns();
		});

		timeline.at(Global::startStep_Chapter3 + horizontalSweepSettings.startStepOffset, [&] {
			if(Global::difficulty == Global::Difficulty::Medium) createHorizontalSweep();
		});

		for (int32 i = 0; i < static_cast<int32>(cogHintDirections.size()); ++i) {
			const int32 direction = cogHintDirections[i];
			const int32 rotationStep = Global::startStep_Chapter3
				+ cogHintSettings.beginStepOffset
				+ cogHintSettings.interval * i;

			timeline.at(rotationStep, [&, direction] {
				for (auto& cog : cogControllers) {
					if (direction < 0) {
						cog->rotateLeft(cogHintSettings.degrees, cogHintSettings.rotationTime, 2);
					}
					else {
						cog->rotateRight(cogHintSettings.degrees, cogHintSettings.rotationTime, 2);
					}
				}
			});
		}

		timeline.at(Global::startStep_Chapter3 + 475, [&] {
			horizontalSweepDisabled = true;
			auto correctMachines = createPressMachine(8, false, 0, &correctPressPattern);
			for (const auto& machine : correctMachines) {
				machine->offset.y = -machine->horizontalDirection * finalPressSettings.hiddenOffset;
				machine->pushDistance = finalPressSettings.hiddenOffset
					+ pressMachineLayout.pressDistance
					- pressShakeDistance;
				machine->requestPush();
			}
		});
	}
}
