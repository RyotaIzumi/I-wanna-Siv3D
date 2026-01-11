#pragma once
#include <Siv3D.hpp>
#include "HitBox.h"

namespace Iwanna {
	class Player {
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
		bool muteki = false; //無敵状態かどうか
		bool roomOutTrue = false;//kid君をroom外にいけるようにする

		//gamemakerで使われる変数
		double hspeed;
		double vspeed;

		//当たり判定用HitBox
		std::shared_ptr<HitBox> hitBox;

		//操作入力用変数
		Input inputLeft;
		Input inputRight;
		Input inputJump;
		Input inputShoot;

		//現在座標
		Vec2 pos;
		//1フレーム前の座標
		Vec2 prevPos;

		//地面に接地しているかどうか
		bool isOnGround = false;
	public:
		Player();

		void update();
		void updateLate();
		void draw() const;

		void playerJump();
		void playerVJump();
		void playerShoot();

		void checkCollisionBlocks(std::shared_ptr<HitBox>& blocks);
		bool getOnGround() const;
	};
}
