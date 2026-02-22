#include "HitBox.h"

namespace Iwanna {
	bool CircleHitBox::intersects(const HitBox& other) const {
		if (auto c = dynamic_cast<const CircleHitBox*>(&other)) {
			return circle.intersects(c->circle);
		}
		else if (auto r = dynamic_cast<const RectHitBox*>(&other)) {
			return circle.intersects(r->rect);
		}
		else if (auto m = dynamic_cast<const SpikeHitBox*>(&other)) {
			return circle.intersects(m->triangle);
		}
		else if (auto m = dynamic_cast<const MikuHitBox*>(&other)) {
			return circle.intersects(m->polygon);
		}
		return false;
	}

	bool RectHitBox::intersects(const HitBox& other) const {
		if (auto r = dynamic_cast<const RectHitBox*>(&other)) {
			return rect.intersects(r->rect);
		}
		else if (auto c = dynamic_cast<const CircleHitBox*>(&other)) {
			return rect.intersects(c->circle);
		}
		else if (auto m = dynamic_cast<const SpikeHitBox*>(&other)) {
			return rect.intersects(m->triangle);
		}
		else if (auto m = dynamic_cast<const MikuHitBox*>(&other)) {
			return rect.intersects(m->polygon);
		}
		return false;
	}

	bool SpikeHitBox::intersects(const HitBox& other) const {
		if (auto r = dynamic_cast<const RectHitBox*>(&other)) {
			return triangle.intersects(r->rect);
		}
		else if (auto c = dynamic_cast<const CircleHitBox*>(&other)) {
			return triangle.intersects(c->circle);
		}
		return false;
	}

	bool MikuHitBox::intersects(const HitBox& other) const {
		if (auto r = dynamic_cast<const RectHitBox*>(&other)) {
			return polygon.intersects(r->rect);
		}
		else if (auto c = dynamic_cast<const CircleHitBox*>(&other)) {
			return polygon.intersects(c->circle);
		}
		return false;
	}
}
