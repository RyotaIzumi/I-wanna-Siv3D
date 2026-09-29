#include "AvoidanceManager.h"

namespace Iwanna {
	namespace {
		// Chapter 9全体で使用する3D空間、足場サイズ、制限時間の設定。
		constexpr int32 Chapter9TimeLimitStep = 5775;
		constexpr double Chapter9WorldScale = (1.0 / 32.0);
		constexpr double Chapter9PlatformSize = 1.8;
		constexpr double Chapter9PlatformHeight = Chapter9PlatformSize;
		constexpr double Chapter9PlatformTop = (Chapter9PlatformHeight * 0.5);

		// Hamilton Pathの各頂点に対応する足場座標。
		const Array<Vec3>& GetChapter9Platforms() {
			static const Array<Vec3> platforms{
				Vec3{ -7.0, 0.0, -5.0 }, Vec3{ -4.0, 0.0, -3.0 }, Vec3{ -6.0, 0.0, 0.0 },
				Vec3{ -3.0, 0.0, 2.0 }, Vec3{ 0.0, 0.0, 0.0 }, Vec3{ 3.0, 0.0, 2.0 },
				Vec3{ 6.0, 0.0, 0.0 }, Vec3{ 4.0, 0.0, -3.0 }, Vec3{ 7.0, 0.0, -5.0 },
			};
			return platforms;
		}

		// 足場間の移動を許可するHamilton Pathの辺。
		const Array<std::pair<int32, int32>>& GetChapter9Edges() {
			static const Array<std::pair<int32, int32>> edges{
				{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 },
				{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 8 },
				{ 1, 4 }, { 2, 4 }, { 4, 7 }, { 5, 7 },
			};
			return edges;
		}

		// 現在の頂点と着地先の足場が辺で接続されているか判定する。
		bool AreChapter9NodesConnected(int32 a, int32 b) {
			return GetChapter9Edges().any([a, b](const auto& edge) {
				return (edge.first == a && edge.second == b)
					|| (edge.first == b && edge.second == a);
			});
		}

		// sprSpikeを四角錐の4側面に貼るための3Dメッシュを生成する。
		const Mesh& GetChapter9SpikeMesh() {
			static const Mesh mesh{ [] {
				const Float3 apex{ 0.0f, 1.15f, 0.0f };
				const Float3 p0{ -0.72f, 0.0f, -0.72f };
				const Float3 p1{ 0.72f, 0.0f, -0.72f };
				const Float3 p2{ 0.72f, 0.0f, 0.72f };
				const Float3 p3{ -0.72f, 0.0f, 0.72f };
				Array<Vertex3D> vertices{
					{ p0, {}, { 0.0f, 1.0f } }, { p1, {}, { 1.0f, 1.0f } }, { apex, {}, { 0.5f, 0.0f } },
					{ p1, {}, { 0.0f, 1.0f } }, { p2, {}, { 1.0f, 1.0f } }, { apex, {}, { 0.5f, 0.0f } },
					{ p2, {}, { 0.0f, 1.0f } }, { p3, {}, { 1.0f, 1.0f } }, { apex, {}, { 0.5f, 0.0f } },
					{ p3, {}, { 0.0f, 1.0f } }, { p0, {}, { 1.0f, 1.0f } }, { apex, {}, { 0.5f, 0.0f } },
				};
				Array<TriangleIndex32> indices{ { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 }, { 9, 10, 11 } };
				return MeshData{ std::move(vertices), std::move(indices) }.computeNormals();
			}() };
			return mesh;
		}
	}

	// Chapter 9開始時、または落下時にHamilton Pathの状態を初期化する。
	void AvoidanceManager::resetChapter9HamiltonPath() {
		chapter9VisitedNodes.assign(GetChapter9Platforms().size(), false);
		chapter9PlatformStates.assign(GetChapter9Platforms().size(), 0);
		chapter9VisitedNodes[0] = true;
		chapter9PlatformStates[0] = 1;
		chapter9PlayerPos = GetChapter9Platforms()[0] + Vec3{ 0.0, Chapter9PlatformTop, 0.0 };
		chapter9PlayerVelocity = Vec3{ 0.0, 0.0, 0.0 };
		chapter9CurrentNode = 0;
		chapter9GroundedPlatform = 0;
		chapter9CanDoubleJump = true;
		chapter9PlayerDirection = Global::Direction::RIGHT;
		chapter9Completed = false;
	}

	// Chapter 9の入力、移動、着地、針化、時間切れを毎STEP更新する。
	void AvoidanceManager::chapter9(const ReplayInputFrame& input) {
		if (chapter9VisitedNodes.size() != GetChapter9Platforms().size()) {
			resetChapter9HamiltonPath();
		}

		// 2D版と同じ移動速度を、32px＝1ワールド単位としてXZ平面へ変換する。
		const double moveSpeed = (3.0 * Chapter9WorldScale);
		Vec2 moveInput{
			static_cast<double>(input.rightPressed) - static_cast<double>(input.leftPressed),
			static_cast<double>(input.downPressed) - static_cast<double>(input.upPressed)
		};
		if (!moveInput.isZero()) {
			moveInput = moveInput.normalized() * moveSpeed;
			if (moveInput.x < 0.0) chapter9PlayerDirection = Global::Direction::LEFT;
			if (0.0 < moveInput.x) chapter9PlayerDirection = Global::Direction::RIGHT;
		}
		chapter9PlayerVelocity.x = moveInput.x;
		chapter9PlayerVelocity.z = -1.0 * moveInput.y;

		// 2D版と同じ通常ジャンプ、2段ジャンプ、可変ジャンプをY軸へ適用する。
		if (input.jumpDown) {
			if (0 <= chapter9GroundedPlatform) {
				chapter9PlayerVelocity.y = (8.5 * Chapter9WorldScale);
				chapter9PlatformStates[chapter9GroundedPlatform] = 2;
				chapter9GroundedPlatform = -1;
			}
			else if (chapter9CanDoubleJump) {
				chapter9PlayerVelocity.y = (7.0 * Chapter9WorldScale);
				chapter9CanDoubleJump = false;
			}
		}
		if (input.jumpUp && 0.0 < chapter9PlayerVelocity.y) {
			chapter9PlayerVelocity.y *= 0.45;
		}
		chapter9PlayerVelocity.y = Max(
			chapter9PlayerVelocity.y - (0.4 * Chapter9WorldScale),
			-9.0 * Chapter9WorldScale);

		const Vec3 previousPos = chapter9PlayerPos;
		chapter9PlayerPos += chapter9PlayerVelocity;

		// プレイヤーが歩いて足場の外へ出た場合も、その足場を針状態へ変更する。
		if (0 <= chapter9GroundedPlatform) {
			const Vec3 platformPos = GetChapter9Platforms()[chapter9GroundedPlatform];
			const double halfSize = (Chapter9PlatformSize * 0.5);
			const bool isAbovePlatform =
				(Abs(chapter9PlayerPos.x - platformPos.x) <= halfSize)
				&& (Abs(chapter9PlayerPos.z - platformPos.z) <= halfSize);
			if (isAbovePlatform) {
				chapter9PlayerPos.y = Chapter9PlatformTop;
				chapter9PlayerVelocity.y = 0.0;
			}
			else {
				chapter9PlatformStates[chapter9GroundedPlatform] = 2;
				chapter9GroundedPlatform = -1;
			}
		}

		// 下降中は、未訪問かつ現在の頂点と接続された足場だけを着地対象にする。
		if (chapter9GroundedPlatform < 0 && chapter9PlayerVelocity.y <= 0.0) {
			const double halfSize = (Chapter9PlatformSize * 0.5);
			for (int32 index = 0; index < static_cast<int32>(GetChapter9Platforms().size()); ++index) {
				if (chapter9PlatformStates[index] == 2
					|| chapter9VisitedNodes[index]
					|| !AreChapter9NodesConnected(chapter9CurrentNode, index)) {
					continue;
				}
				const Vec3 platformPos = GetChapter9Platforms()[index];
				const bool isInsideTop =
					(Abs(chapter9PlayerPos.x - platformPos.x) <= halfSize)
					&& (Abs(chapter9PlayerPos.z - platformPos.z) <= halfSize);
				const bool crossedTop = previousPos.y >= Chapter9PlatformTop
					&& chapter9PlayerPos.y <= Chapter9PlatformTop;
				if (isInsideTop && crossedTop) {
					chapter9PlayerPos.y = Chapter9PlatformTop;
					chapter9PlayerVelocity.y = 0.0;
					chapter9GroundedPlatform = index;
					chapter9CurrentNode = index;
					chapter9CanDoubleJump = true;
					chapter9VisitedNodes[index] = true;
					chapter9PlatformStates[index] = 1;
					chapter9Completed = chapter9VisitedNodes.all([](bool visited) { return visited; });
					break;
				}
			}
		}

		// 落下時は死亡させず、制限時間を維持したままルートだけを初期化する。
		if (chapter9PlayerPos.y < -5.0) {
			resetChapter9HamiltonPath();
		}

		// Chapter 9でプレイヤーを死亡させるのは5775 STEPの時間切れのみ。
		if (Chapter9TimeLimitStep <= step
			&& !chapter9Completed
			&& !gameObjects.player->getIsDead()) {
			gameObjects.player->playerDead();
		}
	}

	// Chapter 9の足場、辺、針、プレイヤースプライト、HUDを描画する。
	void AvoidanceManager::drawChapter9HamiltonPath() const {
		if (activeChapter != 9 || chapter9VisitedNodes.size() != GetChapter9Platforms().size()) {
			return;
		}

		// 深度バッファ付きの描画先を使い、3Dオブジェクトの前後関係を正しく判定する。
		static const MSRenderTexture renderTexture{
			Size{ Global::windowWidth, Global::windowHeight },
			TextureFormat::R8G8B8A8_Unorm_SRGB,
			HasDepth::Yes
		};
		renderTexture.clear(backgroundColor);

		const BasicCamera3D camera{
			renderTexture.size(), 35_deg,
			Vec3{ -1.5, 18.0, -20.0 }, Vec3{ 0.0, 0.0, -1.0 }
		};
		int32 visitedCount = 0;
		{
			const ScopedRenderTarget3D target{ renderTexture };
			// Hamilton Path全体を見渡せるように固定カメラと照明を設定する。
			Graphics3D::SetCameraTransform(camera);
			Graphics3D::SetGlobalAmbientColor(ColorF{ 0.72 });
			Graphics3D::SetSunDirection(Vec3{ 0.4, 1.0, -0.3 }.normalized());

		// グラフの辺を3D空間上の線として描画する。
		for (const auto& [from, to] : GetChapter9Edges()) {
			Line3D{
				GetChapter9Platforms()[from] + Vec3{ 0.0, Chapter9PlatformTop + 0.03, 0.0 },
				GetChapter9Platforms()[to] + Vec3{ 0.0, Chapter9PlatformTop + 0.03, 0.0 }
			}.draw(ColorF{ 0.25, 0.70, 0.90, 0.85 });
		}

		// 各頂点をブロックで描画し、離れた足場には四角錐の針を重ねる。
		for (int32 index = 0; index < static_cast<int32>(GetChapter9Platforms().size()); ++index) {
			if (chapter9VisitedNodes[index]) ++visitedCount;
			const Vec3 platformPos = GetChapter9Platforms()[index];
			const ColorF platformColor = chapter9VisitedNodes[index]
				? ColorF{ 0.62, 1.0, 0.78 } : ColorF{ 0.90 };
			Box{ platformPos, Chapter9PlatformSize, Chapter9PlatformHeight, Chapter9PlatformSize }
				.draw(TextureAsset(U"sprBlock"), platformColor);
			if (chapter9PlatformStates[index] == 2) {
				GetChapter9SpikeMesh().draw(
					platformPos + Vec3{ 0.0, Chapter9PlatformTop, 0.0 }, TextureAsset(U"sprSpike"));
			}
		}

		// 既存の2Dプレイヤースプライトを、Z方向の厚みが0の両面表示として描画する。
		const bool isMoving = (Abs(chapter9PlayerVelocity.x) + Abs(chapter9PlayerVelocity.z)) > 0.001;
		String playerTextureName = U"sprPlayerIdle";
		int32 frameCount = 4;
		if (chapter9GroundedPlatform < 0) {
			playerTextureName = (0.0 < chapter9PlayerVelocity.y) ? U"sprPlayerJump" : U"sprPlayerFall";
			frameCount = 2;
		}
		else if (isMoving) {
			playerTextureName = U"sprPlayerRunning";
		}
		const int32 frameIndex = ((step / 5) % frameCount);
		const TextureRegion playerTexture = TextureAsset(playerTextureName)(frameIndex * 32, 0, 32, 32)
			.mirrored(chapter9PlayerDirection == Global::Direction::LEFT);
		static const Mesh playerPaperMesh{ MeshData::Billboard() };
		{
			const ScopedRenderStates3D renderStates{
				BlendState::Default2D, RasterizerState::SolidCullNone, SamplerState::ClampNearest
			};
			playerPaperMesh.draw(
				chapter9PlayerPos + Vec3{ 0.0, 0.58, 0.0 },
				playerTexture);
		}
			Graphics3D::Flush();
		}
		renderTexture.resolve();
		renderTexture.draw(0, 0);

		// 3D描画を確定した後、訪問数と残り時間を2DのHUDとして描画する。
		const int32 remainingStep = Max(Chapter9TimeLimitStep - step, 0);
		const ColorF timerColor = (remainingStep <= 150)
			? ColorF{ 1.0, 0.28, 0.22 } : ColorF{ 0.92 };
		FontAsset(U"Button")(U"Hamilton Path").draw(Vec2{ 24, 20 }, ColorF{ 0.30, 0.95, 0.78 });
		FontAsset(U"Button")(Format(visitedCount) + U" / " + Format(GetChapter9Platforms().size()))
			.draw(Vec2{ 24, 48 }, ColorF{ 0.92 });
		FontAsset(U"Button")(U"TIME " + ToFixed(remainingStep / static_cast<double>(Global::FPS), 2))
			.draw(Vec2{ 640, 20 }, timerColor);
		if (chapter9Completed) {
			FontAsset(U"Title")(U"PATH COMPLETE").drawAt(Vec2{ 400, 76 }, ColorF{ 0.30, 1.0, 0.72 });
		}
	}
}
