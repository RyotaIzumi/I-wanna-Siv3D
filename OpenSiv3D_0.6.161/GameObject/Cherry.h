#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Cherry : public GameObject {
	public:
		using Behavior = std::function<void(Cherry&, int32)>;

		struct Settings {
			String textureName = U"sprCherry";
			ColorF color = Palette::White;
			Behavior behavior = nullptr;
			bool canDeleteOutOfScreen = true;
		};

	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 10;
		int32 age = 0;
		Behavior behavior;
		String textureName = U"sprCherry";
		ColorF color = Palette::White;
		bool canDeleteOutOfScreen = true;//画面外で消去するかどうかのフラグ

	public:
		double speed = 0;
		double dir = 0;

		bool isDelete = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Cherry();

		void update() override;
		void draw() const override;

		void applySettings(const Settings& settings);
		void setBehavior(const Behavior& newBehavior);
		void setVisual(const String& newTextureName, const ColorF& newColor);
		void setTextureName(const String& newTextureName);
		void setColor(const ColorF& newColor);
		void setCanDeleteOutOfScreen(bool enabled);
		int32 getAge() const;

		void calculateSpeed();
		void checkOutOfScreen();

		void onCollision(GameObject& other) override;
	};
}
