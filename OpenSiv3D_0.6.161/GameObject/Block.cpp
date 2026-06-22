#include "Block.h"

namespace Iwanna {
	Block::Block(String name, Vec2 startPos) {
		textureName = name;
		//GameObject.hの値初期化
		pos = {startPos.x * side, startPos.y * side};
		hitBox = std::make_shared<RectHitBox>(pos, SizeF{ side,side });
		type = ObjectType::Block;

		canPlayerKill = false;
		hasCollide = true;
	}
	void Block::update() {
		if (role == Role::Disposable && visited && hasCollide && collapseFrames > 0) {
			--collapseFrames;
			if (collapseFrames == 0) hasCollide = false;
		}
	}
	void Block::draw() const {
		if (role == Role::Disposable) {
			if (!hasCollide) return;
			const double remaining = (visited ? static_cast<double>(collapseFrames) / 18.0 : 1.0);
			const ColorF color = (visited
				? ColorF{ 1.0, 0.25 + 0.55 * remaining, 0.10, 0.35 + 0.65 * remaining }
				: ColorF{ 1.0, 0.78, 0.12 });
			RectF{ pos, side, side }.draw(color).drawFrame(2, ColorF{ 0.35, 0.18, 0.02 });
			return;
		}
		if (role == Role::Goal) {
			RectF{ pos, side, side }.draw(ColorF{ 0.20, 0.90, 0.38 }).drawFrame(2, ColorF{ 0.02, 0.30, 0.08 });
			Triangle{ Vec2{ pos.x + 8, pos.y + 24 }, Vec2{ pos.x + 8, pos.y - 18 }, Vec2{ pos.x + 30, pos.y - 7 } }
				.draw(ColorF{ 0.30, 1.0, 0.50 });
			return;
		}
		TextureAsset(textureName).draw(pos);
	}
	void Block::onCollision(GameObject& other) {
	}

	//当たり判定の有無を設定
	void Block::setHasCollide(bool b) {
		hasCollide = b;
	}
	//当たり判定の有無を返す
	bool Block::getHasCollide() const {
		return hasCollide;
	}
	void Block::makeDisposable() {
		role = Role::Disposable;
		visited = false;
		collapseFrames = 0;
		hasCollide = true;
	}
	void Block::makeGoal() {
		role = Role::Goal;
		hasCollide = true;
	}
	void Block::activateDisposable() {
		if (role == Role::Disposable && !visited) {
			visited = true;
			collapseFrames = 18;
		}
	}
	bool Block::isDisposable() const { return role == Role::Disposable; }
	bool Block::isVisited() const { return visited; }
	bool Block::isGoal() const { return role == Role::Goal; }
}
