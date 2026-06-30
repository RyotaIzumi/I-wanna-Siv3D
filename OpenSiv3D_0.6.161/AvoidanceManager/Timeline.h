#pragma once
#include <Siv3D.hpp>

namespace Iwanna {

	class Timeline {
	private:
		int32 previousStep = 0;
		int32 currentStep = 0;

	public:
		Timeline(int32 previous, int32 current)
			: previousStep(previous)
			, currentStep(current) {
		}

		void at(int32 eventStep, const std::function<void()>& action) const {
			if (previousStep < eventStep && eventStep <= currentStep) {
				action();
			}
		}

		void every(int32 interval, int32 beginStep, int32 endStep, const std::function<void(int32 localStep)>& action) const {
			if (interval <= 0) {
				return;
			}

			const int32 from = Max(previousStep + 1, beginStep);
			const int32 to = Min(currentStep, endStep);

			for (int32 eventStep = from; eventStep <= to; ++eventStep) {
				if ((eventStep - beginStep) % interval == 0) {
					action(eventStep - beginStep);
				}
			}
		}

		void during(int32 beginStep, int32 endStep, const std::function<void(int32 localStep)>& action) const {
			const int32 from = Max(previousStep + 1, beginStep);
			const int32 to = Min(currentStep, endStep);

			for (int32 eventStep = from; eventStep <= to; ++eventStep) {
				action(eventStep - beginStep);
			}
		}
	};
}
