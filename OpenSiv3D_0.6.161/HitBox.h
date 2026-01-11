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
}
