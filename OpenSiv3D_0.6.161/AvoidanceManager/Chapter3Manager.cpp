#include "AvoidanceManager.h"

namespace {
	constexpr int32 lowerMachine = 0;
	constexpr int32 chapter3LoopFirstStep = 23;
	constexpr int32 chapter3LoopPressStep = 73;
	constexpr int32 chapter3LoopEndStep = 143;
	constexpr int32 chapter3LoopLength = chapter3LoopEndStep - chapter3LoopFirstStep + 1;

	struct PressMachineController {
		Vec2 offset = Vec2{ 0,0 };
		Iwanna::EasingMove move;
		int32 pressRequest = 0;
		int32 horizontalDirection = -1;

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
		const std::shared_ptr<PressMachineController>& controller) {

		struct State {
			Vec2 localHome;
			Vec2 pressHome;
			Vec2 phaseStart;
			Vec2 phaseEnd;
			int32 upDown = lowerMachine;
			int32 phase = 0;
			int32 timer = 0;
			int32 seenPressRequest = 0;
		};

		auto state = std::make_shared<State>();
		state->localHome = localHome;
		state->upDown = upDownPattern;

		return [state, controller](Iwanna::Cherry& self, int32) {
			constexpr int32 shakeTime = 23;
			constexpr int32 moveTime = 20;
			constexpr double shakeWidthX = 1.0;
			constexpr double shakeWidthY = 50.0;
			constexpr double elevatingWidth = 200.0;

			const Vec2 currentHome = state->localHome + controller->offset;
			const double sign = (state->upDown == lowerMachine) ? 1.0 : -1.0;

			if ((controller->horizontalDirection < 0 && currentHome.x < -100.0)
				|| (controller->horizontalDirection > 0 && currentHome.x > Global::windowWidth + 100.0)) {
				self.isDelete = true;
				return;
			}

			if (state->phase == 0) {
				self.pos = currentHome;

				if (state->seenPressRequest != controller->pressRequest) {
					state->seenPressRequest = controller->pressRequest;
					state->pressHome = currentHome;
					state->timer = 0;
					state->phase = 1;
				}

				return;
			}

			switch (state->phase) {
			case 1: {
				const double t = state->timer / static_cast<double>(shakeTime);
				const double e = bezierEase(t, 0.5, 0.99);
				self.pos.x = state->pressHome.x - shakeWidthX * Math::Sin(Math::ToRadians(720.0 * e));
				self.pos.y = state->pressHome.y + sign * shakeWidthY * e;

				++state->timer;
				if (state->timer > shakeTime) {
					state->phaseStart = self.pos;
					state->phaseEnd = self.pos + Vec2{ 0, -sign * elevatingWidth };
					state->timer = 0;
					state->phase = 2;
				}
				break;
			}

			case 2: {
				const double t = state->timer / static_cast<double>(moveTime);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * easeInOut(t);

				++state->timer;
				if (state->timer > moveTime) {
					state->phaseStart = self.pos;
					state->phaseEnd = state->pressHome;
					state->timer = 0;
					state->phase = 3;
				}
				break;
			}

			case 3: {
				const double t = state->timer / static_cast<double>(moveTime);
				self.pos = state->phaseStart + (state->phaseEnd - state->phaseStart) * easeInOut(t);

				++state->timer;
				if (state->timer > moveTime) {
					state->timer = 0;
					state->phase = 0;
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
}

namespace Iwanna {
	// step : 1320 - 1799
	void AvoidanceManager::chapter3() {
		Timeline timeline(previousStep, step);

		timeline.at(Global::startStep_Chapter3, [&] {
			pressMachines.clear();
			cogControllers.clear();
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

		const auto createPressMachine = [&](double baseX, double lowerBaseY) {
			constexpr int32 widthNum = 50;
			constexpr int32 heightNum = 2;
			constexpr int32 protNum = 10;
			constexpr int32 protHeightNum = 4;
			constexpr int32 cellSize = 16;
			constexpr int32 protWidth = widthNum / protNum;

			Array<int32> downProtPattern(protNum);
			Array<int32> upProtPattern(protNum);

			for (int32 i = 0; i < protNum; ++i) {
				downProtPattern[i] = Random(0, 1);
				upProtPattern[i] = downProtPattern[i] + 1;
			}

			const int32 safeIndex = Random(3, 5);
			downProtPattern[safeIndex] = 0;
			upProtPattern[safeIndex] = 0;

			const auto createMachineCherry = [&](const Vec2& localPos, int32 upDownPattern, const std::shared_ptr<PressMachineController>& controller) {
				auto cherry = std::make_shared<Cherry>();
				cherry->pos = localPos + controller->offset;
				cherry->applySettings(Cherry::Settings{
					.textureName = U"sprCherryAllWhite",
					.color = ColorF(0.08, 0.08, 0.09),
					.behavior = makePressMachineBehavior(localPos, upDownPattern, controller),
					.canDeleteOutOfScreen = false,
				});
				createCherry(cherry);
			};

			const auto createMachineSide = [&](int32 upDownPattern, double sideBaseX, double argumentY, int32 horizontalDirection) {
				auto controller = std::make_shared<PressMachineController>();
				controller->horizontalDirection = horizontalDirection;
				pressMachines << controller;

				for (int32 j = 0; j < heightNum; ++j) {
					const double y = argumentY - j * cellSize;

					for (int32 i = 0; i < widthNum; ++i) {
						const double x = sideBaseX + i * cellSize;
						createMachineCherry(Vec2{ x, y }, upDownPattern, controller);
					}
				}

				for (int32 i = 0; i < protHeightNum; ++i) {
					const double y = (upDownPattern == lowerMachine)
						? argumentY - i * cellSize - heightNum * cellSize
						: argumentY + i * cellSize + cellSize;

					for (int32 k = 0; k < protNum; ++k) {
						const bool shouldCreate = (upDownPattern == lowerMachine)
							? (downProtPattern[k] == 1)
							: (upProtPattern[k] == 1);

						if (!shouldCreate) {
							continue;
						}

						for (int32 j = 0; j < protWidth; ++j) {
							const double x = sideBaseX + protWidth * cellSize * k + j * cellSize;
							createMachineCherry(Vec2{ x, y }, upDownPattern, controller);
						}
					}
				}
			};

			createMachineSide(lowerMachine, baseX, lowerBaseY, -1);
			createMachineSide(1, -baseX, lowerBaseY - 400.0, 1);
		};

		timeline.at(Global::startStep_Chapter3 + 1, [&] {
			createCog(Vec2{ 400,304 }, 8);
		});

		timeline.every(
			chapter3LoopLength,
			Global::startStep_Chapter3 + chapter3LoopFirstStep,
			Global::startStep_Chapter4 - 1,
			[&](int32) {
				for (auto& machine : pressMachines) {
					machine->scrMoveEasing(2, machine->offset + Vec2{ 800.0 * machine->horizontalDirection,0 }, 50);
				}
				for (auto& cog : cogControllers) {
					cog->rotateRight(90.0, 50, 2);
				}

				createPressMachine(800.0, 516.0);
			});

		timeline.every(
			chapter3LoopLength,
			Global::startStep_Chapter3 + chapter3LoopPressStep,
			Global::startStep_Chapter4 - 1,
			[&](int32) {
				for (auto& machine : pressMachines) {
					machine->requestPress();
				}
			});
	}
}
