#pragma once
#include <Siv3D.hpp>

namespace Iwanna {
	class AvoidanceManager {
	private:
		int32 step = 0;
	public:
		AvoidanceManager();

		void update();
		void setStep(int32 newStep);
	};
}
