#include "Player.h"
#include "../Audio/AudioAsset.h"

namespace Iwanna{
	Player::Player() {
		frozen = false; //操作を受け付けるかどうか
		frozen2 = false; //↑の予備
		jump = 8.5; //１段目ジャンプ力
		jump2 = 7; //２段目ジャンプ力
		djump = true; //２段ジャンプできるかどうか
		maxSpeed = 3; //横方向速度の最大値
		gravity = 0.4; //重力の値
		maxVspeed = 9; //縦方向速度(主に落下速度)の最大値
		image_speed = 0.2; //アニメーション再生速度
		muteki = false; //無敵状態かどうか
		roomOutTrue = false;//kid君をroom外にいけるようにする

		//GameObject.hの値初期化
		pos = Vec2(100, 100);
		hitBox = std::make_shared<RectHitBox>(Vec2(0, 0), hitBoxSize);
		type = ObjectType::Player;

		//アニメーションデータの登録
		//(アクション名,フレーム数,各フレーム再生時間,ループするかどうか(省略可), 左右差分があるか(省略可))
		spriteSystem = SpriteSystem(32, 32);
		spriteSystem.addSprite(SpriteAction::PLAYER_WAIT, SpriteData(U"sprPlayerIdle", 4, 0.15,true,false));
		spriteSystem.addSprite(SpriteAction::PLAYER_RUN, SpriteData(U"sprPlayerRunning", 4, 0.1,true,false));
		spriteSystem.addSprite(SpriteAction::PLAYER_JUMP, SpriteData(U"sprPlayerJump", 2, 0.1,true,false));
		spriteSystem.addSprite(SpriteAction::PLAYER_FALL, SpriteData(U"sprPlayerFall", 2, 0.1,true,false));

		//初期の向き
		direction = Global::Direction::RIGHT;

		hspeed = 0.0;
		vspeed = 0.0;
	}

	void Player::update() {

		hspeed = 0.0;
		isChanedActionWait = false;

		if (Global::inputLeft.pressed()) playerMoveLeft();
		if (Global::inputRight.pressed()) playerMoveRight();
		

		if (!frozen) {
			if (Global::inputShoot.down()) playerShoot();
			if (Global::inputJump.down()) playerJump();
			if (Global::inputJump.up()) playerVJump();
		}

		// 重力反映
		vspeed += gravity;

		// ジャンプ時アニメーション反映
		if (!isOnGround) {
			if (vspeed < -0.05) {
				spriteSystem.setSprite(SpriteAction::PLAYER_JUMP);
				isChanedActionWait = true;
			}
			if (vspeed > 0.05) {
				spriteSystem.setSprite(SpriteAction::PLAYER_FALL);
				isChanedActionWait = true;
			}
		}

		if(!isChanedActionWait)
			spriteSystem.setSprite(SpriteAction::PLAYER_WAIT);

		isOnGround = false;
	}

	void Player::updateLate() {
		// 移動
		pos.x += hspeed;
		pos.y += vspeed;

		hitBox->setPos(pos);
	}

	void Player::draw() const {
		hitBox->draw(Palette::Red);
		
		TextureRegion texture = spriteSystem.getTextureRegion(direction);
		texture.drawAt(pos.x,pos.y - 6);
	}

	void Player::playerMoveLeft() {
		hspeed = -maxSpeed;
		direction = Global::Direction::LEFT;
		spriteSystem.setSprite(SpriteAction::PLAYER_RUN);
		isChanedActionWait = true;
	}

	void Player::playerMoveRight() {
		hspeed = maxSpeed;
		direction = Global::Direction::RIGHT;
		spriteSystem.setSprite(SpriteAction::PLAYER_RUN);
		isChanedActionWait = true;
	}

	void Player::playerJump() {
		if (isOnGround) {
			vspeed = -jump;
			djump = true;
			AudioAsset(Sound::JUMP).playOneShot();
			isOnGround = false;
		}
		else if (djump) {
			vspeed = -jump2;
			djump = false;
			AudioAsset(Sound::DJUMP).playOneShot();
		}
	}

	void Player::playerVJump() {
		if (vspeed < 0) {
			vspeed *= 0.45;
		}
	}

	void Player::playerShoot() {
		AudioAsset(Sound::SHOOT).playOneShot();
	}


	void Player::onCollision(GameObject& other)
	{
		// ブロック衝突
		if (other.type == ObjectType::Block)
		{
			// --- 横方向 予測衝突 ---
			if (hspeed != 0)
			{
				RectF nextHitBox = RectF(Arg::center(hitBox->getCenterPos().x + hspeed, hitBox->getCenterPos().y), hitBoxSize.x, hitBoxSize.y - 4);

				if (nextHitBox.intersects(*other.hitBox->getRect()))
				{
					hspeed = 0; // 移動キャンセル
				}
			}

			// --- 縦方向 予測衝突 ---
			if (vspeed != 0)
			{
				//方向によって当たり判定の位置、大きさを変える
				RectF nextHitBox;
				if(vspeed > 0) nextHitBox = RectF(Arg::center(hitBox->getCenterPos().x, hitBox->getCenterPos().y + vspeed + hitBoxSize.y / 2), hitBoxSize.x - 3, 1);
				if(vspeed < 0) nextHitBox = RectF(Arg::center(hitBox->getCenterPos().x, hitBox->getCenterPos().y + vspeed - hitBoxSize.y / 2 + 2), hitBoxSize.x - 3, 1);

				if (nextHitBox.intersects(*other.hitBox->getRect()))
				{
					if (vspeed > 0) {
						pos.y = other.hitBox->top().y - 10;
						djump = true;
						isOnGround = true;
					}
					vspeed = 0;
				}
			}
		}
	}


	bool Player::getOnGround() const {
		return isOnGround;
	}
}
