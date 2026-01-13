#include "Player.h"
#include "Sprite/SpriteAsset.h"

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

		hitBox = std::make_shared<RectHitBox>(Vec2(0, 0), hitBoxSize);

		//アニメーションデータの登録
		//(アクション名,フレーム数,各フレーム再生時間,ループするかどうか(省略可), 左右差分があるか(省略可))
		spriteSystem = SpriteSystem(32, 32);
		spriteSystem.addAnimation(AnimationAction::PLAYER_WAIT, SpriteData(U"sprPlayerIdle", 4, 0.2,true,false));

		//初期の向き
		direction = Global::Direction::RIGHT;

		inputLeft = KeyLeft;
		inputRight = KeyRight;
		inputJump = KeyShift;
		inputShoot = KeyZ;

		hspeed = 0.0;
		vspeed = 0.0;

		//player初期座標
		pos = Vec2(100, 100);
	}

	void Player::update() {

		hspeed = 0.0;
		if (KeyLeft.pressed())  hspeed = -maxSpeed;
		if (KeyRight.pressed()) hspeed = maxSpeed;

		if (!frozen) {
			if (inputShoot.down()) playerShoot();
			if (inputJump.down()) playerJump();
			if (inputJump.up()) playerVJump();
		}

		// 重力
		vspeed += gravity;

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
		//TextureAsset(U"sprPlayerIdle")(0,0,32,32).drawAt(pos);
		TextureRegion texture = spriteSystem.getTextureRegion(direction);
		texture.drawAt(pos);
	}

	void Player::playerJump() {
		if (isOnGround) {
			vspeed = -jump;
			djump = true;
			isOnGround = false;
		}
		else if (djump) {
			vspeed = -jump2;
			djump = false;
		}
	}

	void Player::playerVJump() {
		if (vspeed < 0) {
			vspeed *= 0.45;
		}
	}

	void Player::playerShoot() {
	}

	void Player::checkCollisionBlocks(std::shared_ptr<HitBox>& block)
	{

		// --- 横方向 予測衝突 ---
		if (hspeed != 0)
		{
			RectF nextHitBox = RectF(Arg::center(hitBox->getCenterPos().x + hspeed, hitBox->getCenterPos().y), hitBoxSize.x, hitBoxSize.y / 4);

			if (nextHitBox.intersects(*block->getRect()))
			{
				hspeed = 0; // 移動キャンセル
			}
		}

		// --- 縦方向 予測衝突 ---
		if (vspeed != 0)
		{
			RectF nextHitBox = RectF(Arg::center(hitBox->getCenterPos().x, hitBox->getCenterPos().y + vspeed), hitBoxSize);

			if (nextHitBox.intersects(*block->getRect()))
			{
				if (vspeed > 0) {
					pos.y = block->top().y - 10;
					isOnGround = true;
				}
				vspeed = 0;
			}
		}
	}


	bool Player::getOnGround() const {
		return isOnGround;
	}
}
