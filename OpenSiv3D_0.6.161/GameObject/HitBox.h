#pragma once
#include <Siv3D.hpp>

namespace Iwanna {
	class HitBox {
	public:
		virtual ~HitBox() = default;

		// 描画（デバッグ用）
		virtual void draw(const ColorF& color = Palette::White) const = 0;
		// 任意の他のHitBoxとの交差判定
		virtual bool intersects(const HitBox& other) const = 0;
		// 座標を動かす
		virtual void setPos(const Vec2& pos) = 0;
		//中心座標を取得
		virtual Vec2 getCenterPos() = 0;

		// Rect / Circle を取得（持っていない型は nullptr を返す）
		virtual const RectF* getRect() const { return nullptr; }
		virtual const Circle* getCircle() const { return nullptr; }

		//上下左右の判定の端を取得
		virtual Vec2 left() = 0;
		virtual Vec2 right() = 0;
		virtual Vec2 top() = 0;
		virtual Vec2 bottom() = 0;
	};

	class CircleHitBox : public HitBox {
	public:
		Circle circle;

		CircleHitBox(const Vec2& pos, double r)
			: circle(pos, r) {
		}

		void draw(const ColorF& color = Palette::White) const override {
			circle.draw(color);
		}

		bool intersects(const HitBox& other) const override;

		void setPos(const Vec2& pos) override {
			circle.setPos(pos);
		}

		Vec2 getCenterPos() override {
			return circle.center;
		}

		const Circle* getCircle() const override {
			return &circle;
		}


		Vec2 left() override {
			return circle.left();
		}

		Vec2 right() override {
			return circle.right();
		}

		Vec2 top() override {
			return circle.top();
		}

		Vec2 bottom() override {
			return circle.bottom();
		}
	};

	class RectHitBox : public HitBox {
	public:
		RectF rect;

		RectHitBox(const Vec2& pos, const SizeF& size)
			: rect(pos, size) {
		}

		RectHitBox(double x, double y, double w, double h)
			: rect(x, y, w, h) {
		}

		void draw(const ColorF& color = Palette::White) const override {
			rect.draw(color);
		}

		bool intersects(const HitBox& other) const override;

		void setPos(const Vec2& pos) override {
			rect.setPos(Arg::center(pos));
		}

		Vec2 getCenterPos() override {
			return rect.center();
		}

		const RectF* getRect() const override {
			return &rect;
		}


		Vec2 left() override {
			return rect.leftCenter();
		}

		Vec2 right() override {
			return rect.rightCenter();
		}

		Vec2 top() override {
			return rect.topCenter();
		}

		Vec2 bottom() override {
			return rect.bottomCenter();
		}
	};

	class SpikeHitBox : public HitBox {
	public:
		Triangle triangle;
		int32 spikeDir;
		Vec2 basePos;// 一ブロックの左上

		SpikeHitBox(const Vec2& pos, const int32& dir) {
			spikeDir = dir;
			setPos(pos);
		}

		void draw(const ColorF& color = Palette::White) const override {
			triangle.draw(color);
		}

		bool intersects(const HitBox& other) const override;

		void setPos(const Vec2& pos) override {
			switch (spikeDir) {
			case 0://上向き
				triangle.set({
					Vec2{ pos.x + 16,  pos.y },
					Vec2{ pos.x + 32, pos.y + 32 },
					Vec2{ pos.x, pos.y + 32  }
				});
				break;
			case 1://左向き
				triangle.set({
					Vec2{ pos.x + 32,  pos.y },
					Vec2{ pos.x + 32, pos.y + 32 },
					Vec2{ pos.x, pos.y + 16  }
				});
				break;
			case 2://下向き
				triangle.set({
					Vec2{ pos.x,  pos.y },
					Vec2{ pos.x + 32, pos.y },
					Vec2{ pos.x + 16, pos.y + 32  }
				});
				break;
			case 3://右向き
				triangle.set({
					Vec2{ pos.x,  pos.y },
					Vec2{ pos.x + 32, pos.y + 16 },
					Vec2{ pos.x, pos.y + 32}
				});
				break;
			}
		}

		Vec2 getCenterPos() override {
			return basePos;
		}

		Vec2 left() override {
			return { basePos.x, basePos.y + 16 };
		}

		Vec2 right() override {
			return { basePos.x + 32, basePos.y + 16 };
		}

		Vec2 top() override {
			return { basePos.x + 16, basePos.y };
		}

		Vec2 bottom() override {
			return { basePos.x + 16, basePos.y + 32 };
		}
	};

	class MikuHitBox : public HitBox {
	public:
		Polygon polygon;
		Vec2 basePos;// 右手当たり

		MikuHitBox(const Vec2& pos) {
			basePos = { pos.x - 96, pos.y };
			polygon = Polygon({
				Vec2{ basePos.x + 56,  basePos.y - 220 },
				Vec2{ basePos.x + 117, basePos.y - 228 },
				Vec2{ basePos.x + 250, basePos.y + 31  },
				Vec2{ basePos.x + 144, basePos.y + 218 },
				Vec2{ basePos.x + 7,   basePos.y + 218 },
				Vec2{ basePos.x + 2,   basePos.y + 175 },
				Vec2{ basePos.x - 47,  basePos.y + 148 },
				Vec2{ basePos.x - 57,  basePos.y + 75  }
			});
		}

		void draw(const ColorF& color = Palette::White) const override {
			polygon.draw(color);
		}

		bool intersects(const HitBox& other) const override;

		void setPos(const Vec2& pos) override {
			basePos = pos;
		}

		Vec2 getCenterPos() override {
			return basePos;
		}

		Vec2 left() override {
			return { basePos.x - 57, basePos.y };
		}

		Vec2 right() override {
			return { basePos.x + 250, basePos.y };
		}

		Vec2 top() override {
			return { basePos.x, basePos.y - 228 };
		}

		Vec2 bottom() override {
			return { basePos.x, basePos.y + 218 };
		}
	};
}
