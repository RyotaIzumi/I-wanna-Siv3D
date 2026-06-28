#include "AvoidanceManager.h"

namespace {
	constexpr int32 lowerMachine = 0;
	constexpr int32 upperMachine = 1;
	constexpr int32 randomMachine = -1;
	constexpr int32 chapter3LoopFirstStep = 23;
	constexpr int32 chapter3LoopPressStep = 73;
	constexpr int32 chapter3LoopEndStep = 143;
	constexpr int32 chapter3LoopLength = chapter3LoopEndStep - chapter3LoopFirstStep + 1;
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

	struct SpecialPressState {
		bool moveFinished = false;
	};

	struct PressMachineController {
		Vec2 offset = Vec2{ 0,0 };
		Iwanna::EasingMove move;
		std::shared_ptr<SpecialPressState> specialPressState;
		int32 pressRequest = 0;
		int32 specialPressRequest = 0;
		int32 horizontalDirection = -1;
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

	struct CogController {
		Vec2 center = Vec2{ 400,304 };
		double angle = 0.0;
		Iwanna::EasingValue rotation;

		void update() {
			if (rotation.isActive()) {
				angle = rotation.update();
			}
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
						state->phaseEnd = state->pressHome;
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
					state->phase = 0;
				}
				break;
			}

			case 7: {// special phase wait
				self.pos = state->phaseStart;

				if (controller->isSpecialMoveFinished()) {
					state->phaseStart = self.pos;
					state->phaseEnd = state->pressHome;
					state->timer = 0;
					state->phase = 3;
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
			const double rad = Math::ToRadians(controller->angle);
			const double cosA = Math::Cos(rad);
			const double sinA = Math::Sin(rad);
			const Vec2 rotated{
				localOffset.x * cosA - localOffset.y * sinA,
				localOffset.x * sinA + localOffset.y * cosA
			};

			self.pos = controller->center + rotated;
		};
	}

	Array<std::shared_ptr<PressMachineController>> pressMachines;
	Array<std::shared_ptr<CogController>> cogControllers;
	int32 selectedSpecialMovingSide = lowerMachine;
	int32 selectedMarkerPress = 1;
}

namespace Iwanna {
	// step : 1320 - 1799
	void AvoidanceManager::chapter3() {
		Timeline timeline(previousStep, step);

		timeline.at(Global::startStep_Chapter3, [&] {
			pressMachines.clear();
			cogControllers.clear();
			selectedSpecialMovingSide = (pressMachineLayout.specialMovingSide == randomMachine)
				? Random(lowerMachine, upperMachine)
				: pressMachineLayout.specialMovingSide;
			selectedMarkerPress = Random(1, 2);
		});

		for (auto& machine : pressMachines) {
			machine->update();
		}
		for (auto& cog : cogControllers) {
			cog->update();
		}
		pressMachines.remove_if([](const auto& machine) {
			return machine->canRemove();
		});

		const auto createCog = [&](const Vec2& center, int32 teethNum) {
			constexpr int32 cirNum = 4;
			constexpr int32 outCirNum = 4;
			constexpr int32 minCirNum = 15;
			constexpr int32 addCirNum = 5;
			constexpr double startRadius = 40.0;
			constexpr double addRadius = 10.0;

			auto controller = std::make_shared<CogController>();
			controller->center = center;
			cogControllers << controller;

			const auto createCogCherry = [&](const Vec2& localOffset) {
				auto cherry = std::make_shared<Cherry>();
				cherry->pos = center + localOffset;
				cherry->applySettings(Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = ColorF(1.0, 1.00, 1.00),
					.behavior = makeCogBehavior(localOffset, controller),
					.canDeleteOutOfScreen = false,
					.depth = DrawDepth::Cherry - 1.0,
				});
				cherry->canPlayerKill = false;
				createCherry(cherry);
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

		const auto createPressMachine = [&](double baseX, bool createWhiteMarker) {
			const auto& layout = pressMachineLayout;
			const int32 toothWidth = layout.widthNum / layout.toothSegmentNum;
			auto specialPressState = std::make_shared<SpecialPressState>();
			const int32 specialMovingSide = selectedSpecialMovingSide;

			Array<int32> downProtPattern(layout.toothSegmentNum);
			Array<int32> upProtPattern(layout.toothSegmentNum);

			for (int32 i = 0; i < layout.toothSegmentNum; ++i) {
				downProtPattern[i] = Random(0, 1);
				upProtPattern[i] = downProtPattern[i] + 1;
			}

			const int32 safeIndex = Random(3, 5);
			downProtPattern[safeIndex] = 0;
			upProtPattern[safeIndex] = 0;

			const auto createMachineCherry = [&](const Vec2& localPos, int32 upDownPattern, const ColorF& color, double depth, double scale, const std::shared_ptr<PressMachineController>& controller) {
				auto cherry = std::make_shared<Cherry>();
				cherry->pos = localPos + controller->offset;
				cherry->applySettings(Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = color,
					.behavior = makePressMachineBehavior(localPos, upDownPattern, controller, layout.pressDistance),
					.canDeleteOutOfScreen = false,
					.depth = depth,
					.scale = scale,
				});
				createCherry(cherry);
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
							? (downProtPattern[k] == 1)
							: (upProtPattern[k] == 1);

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
			};

			createMachineSide(lowerMachine, baseX, layout.lowerToothRootY, -1);
			createMachineSide(upperMachine, -baseX + layout.cellSize, layout.upperToothRootY, 1);
		};

		timeline.at(Global::startStep_Chapter3 + 1, [&] {
			createCog(Vec2{ 400,304 }, 8);
			createCog(Vec2{ 170,304 }, 8);
			createCog(Vec2{ 630,304 }, 8);
			createPressMachine(8, false);
			createPressMachine(808.0, selectedMarkerPress == 1);
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
				createPressMachine(808.0, incomingPressNumber == selectedMarkerPress);
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

	}
}
