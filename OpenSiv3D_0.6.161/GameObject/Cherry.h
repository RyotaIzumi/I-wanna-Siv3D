#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	enum class CherryEffect {
		None,
		FadeIn,
		ScaleIn,
		FadeOut,
		ScaleOut,
	};

	class Cherry : public GameObject {
	public:
		using Behavior = std::function<void(Cherry&, int32)>;

		struct Settings {
			String textureName = U"sprCherry";
			ColorF color = Palette::White;
			Behavior behavior = nullptr;
			bool canDeleteOutOfScreen = true;
			bool canPlayerKill = true;
			double depth = DrawDepth::Cherry;
			double scale = 1.0;
			double alpha = 1.0;
			CherryEffect appearanceEffect = CherryEffect::None;
			int32 appearanceDuration = 30;
		};

	private:
		//当たり判定サイズ(半径)
		int32 hitBoxSize = 10;
		int32 age = 0;
		Behavior behavior;
		String textureName = U"sprCherry";
		ColorF color = Palette::White;
		double scale = 1.0;
		CherryEffect appearanceEffect = CherryEffect::None;
		int32 appearanceElapsed = 0;
		int32 appearanceDuration = 1;
		double appearanceTargetScale = 1.0;
		double appearanceTargetAlpha = 1.0;
		bool canPlayerKillAtFullAlpha = true;
		bool canDeleteOutOfScreen = true;//画面外で消去するかどうかのフラグ

		void updateAppearanceEffect();

	public:
		double speed = 0;
		double dir = 0;

		bool isDelete = false;//消去用フラグ
		bool isOutOfScreen = false;//画面外判定用フラグ

		Cherry();
		void reset(const Vec2& newPos, const Settings& settings);
		void deactivate();

		void update() override;
		void draw() const override;

		void applySettings(const Settings& settings);
		void setBehavior(const Behavior& newBehavior);
		void setVisual(const String& newTextureName, const ColorF& newColor);
		void setTextureName(const String& newTextureName);
		void setColor(const ColorF& newColor);
		void setScale(double newScale);
		void setEffect(CherryEffect effect, int32 duration = 30);
		void setAppearanceEffect(CherryEffect effect, int32 duration = 30);
		void setCanDeleteOutOfScreen(bool enabled);
		int32 getAge() const;

		void calculateSpeed();
		void checkOutOfScreen();

		void onCollision(GameObject& other) override;
	};
}
