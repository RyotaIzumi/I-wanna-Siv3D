#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"
#include "../Sprite/SpriteSystem.h"
#include "../Global.h"
#include "GameObject.h"

namespace Iwanna {
	class Player : public GameObject {
	private://以下値は元のI wanna の値と同じ
		bool frozen = false; //操作を受け付けるかどうか
		bool frozen2 = false; //↑の予備
		double jump = 8.5; //１段目ジャンプ力
		double jump2 = 7; //２段目ジャンプ力
		double djump = true; //２段ジャンプできるかどうか
		double maxSpeed = 3; //横方向速度の最大値
		double gravity = 0.4; //重力の値
		double maxVspeed = 9; //縦方向速度(主に落下速度)の最大値
		double image_speed = 0.2; //アニメーション再生速度
		bool isMuteki = false; //無敵状態かどうか
		bool roomOutTrue = false;//kid君をroom外にいけるようにする
		bool isDead = false; //死亡状態かどうか
		bool isGenerateBullet = false; //弾生成フラグ

		//player当たり判定サイズ
		SizeF hitBoxSize{ 11,21 };

		//アニメーション管理用変数
		SpriteSystem spriteSystem;
		//向き管理用変数
		Global::Direction direction;

		//地面に接地しているかどうか
		bool isOnGround = false;
		//待機状態以外のアニメーション設定がなされたかどうか
		bool isChanedActionWait = false;

	public:
		Player();

		void update() override;
		void updateLate();
		void draw() const override;

		void playerMoveLeft();
		void playerMoveRight();
		void playerJump();
		void playerVJump();
		void playerShoot();
		void playerDead();

		Vec2 snappedPos(Vec2 p);

		void onCollision(GameObject& other) override;
		bool getOnGround() const;
		bool getIsDead() const;

		void setIsGenerateBullet(bool value);
		bool getIsGenerateBullet() const;
		Global::Direction getDirection() const;
		void setIsMuteki(bool value);
		bool getIsMuteki() const;
	};
}
