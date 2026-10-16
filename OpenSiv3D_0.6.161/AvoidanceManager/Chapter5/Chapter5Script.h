#pragma once

namespace Iwanna {
	class AvoidanceManager;
	class Timeline;

	// Owns the chapter schedule; patterns and individual movement live elsewhere.
	class Chapter5Script {
	public:
		void update(AvoidanceManager& manager, const Timeline& timeline) const;
	};
}
