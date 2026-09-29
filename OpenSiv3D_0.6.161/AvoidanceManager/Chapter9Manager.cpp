#include "AvoidanceManager.h"

namespace Iwanna {
	namespace {
		// Chapter 9全体で使用する3D空間、足場サイズ、制限時間の設定。
		constexpr int32 Chapter9TimeLimitStep = 5775;
		constexpr double Chapter9WorldScale = (1.0 / 32.0);
		constexpr double Chapter9PlatformSize = 1.8;
		constexpr double Chapter9PlatformHeight = Chapter9PlatformSize;
		constexpr double Chapter9PlatformTop = (Chapter9PlatformHeight * 0.5);

		// カメラのX座標は±1.5を往復する。周期は片道ではなく1往復の秒数。
		constexpr double Chapter9CameraAmplitude = 1.5;
		constexpr double Chapter9CameraPeriodSeconds = 10.0;
		static_assert(Chapter9CameraPeriodSeconds > 0.0, "Camera period must be positive");

		constexpr double Chapter9PlayerShadowRadius = 0.38;
		constexpr double Chapter9PlayerShadowOpacity = 0.55;

		// 生成設定。格子は初期配置にだけ使用し、生成後に連続座標で配置を崩す。
		// ブロック数は1～列数×行数。既定の範囲は現在のカメラ内に収まる。
		constexpr int32 Chapter9BlockCount = 10;
		constexpr int32 Chapter9GridColumns = 5;
		constexpr int32 Chapter9GridRows = 3;
		constexpr double Chapter9GridSpacing = 3.5;
		constexpr double Chapter9PositionJitter = 0.18; // 各軸の位置の揺らぎ
		constexpr int32 Chapter9ScatterPasses = 160; // 全ブロックを動かす試行回数
		constexpr double Chapter9ScatterStep = 0.8; // 1回の試行で各軸を動かす最大距離
		constexpr double Chapter9PlatformGap = 1.8; // 希望する最小の隙間。ジャンプ可能な上限に自動補正
		constexpr double Chapter9MaxEdgeLength = 4.5; // 軸方向の中心間距離上限。斜めはブロックの幅を加味する
		constexpr double Chapter9ExtraEdgeProbability = 0.7; // 分岐確率（0～1）
		constexpr double Chapter9DiagonalOffset = 1.5; // 初期配置の行ずらし幅
		constexpr int32 Chapter9SearchBudget = 20000;
		constexpr bool Chapter9RequireTrap = true; // 4ブロック以上なら詰む選択肢を必ず作る
		constexpr int32 Chapter9TrapLayoutAttempts = 24; // 条件を満たす配置の試行上限
		const Vec3 Chapter9StartPosition{ -7.0, 0.0, -5.0 };
		static_assert(Chapter9GridColumns > 0 && Chapter9GridRows > 0);
		static_assert(Chapter9BlockCount > 0 && Chapter9BlockCount <= Chapter9GridColumns * Chapter9GridRows);
		static_assert(Chapter9PositionJitter >= 0.0);
		static_assert(Chapter9ScatterPasses >= 0 && Chapter9ScatterStep >= 0.0 && Chapter9PlatformGap >= 0.0);
		static_assert(Chapter9GridSpacing > 0.0);
		static_assert(Chapter9DiagonalOffset > 0.0);
		static_assert(0.0 < Chapter9MaxEdgeLength && Chapter9MaxEdgeLength <= 4.5);

		static_assert(0.0 <= Chapter9ExtraEdgeProbability && Chapter9ExtraEdgeProbability <= 1.0);

		// 隙間設定から生成用の値を決める。0.1は配置を崩せる余裕として残す。
		// MaxEdgeLength=4.5、PlatformSize=1.8の場合、隙間の上限は2.6。
		static_assert(Chapter9MaxEdgeLength > Chapter9PlatformSize + 0.1);
		const double Chapter9EffectiveGap = Min(Chapter9PlatformGap,
			Chapter9MaxEdgeLength - Chapter9PlatformSize - 0.1);
		const double Chapter9EffectiveSpacing = Clamp(
			Max(Chapter9GridSpacing, Chapter9PlatformSize + Chapter9EffectiveGap + 2.0 * Chapter9PositionJitter),
			Chapter9PlatformSize + Chapter9EffectiveGap, Chapter9MaxEdgeLength - 0.05);
		const double Chapter9EffectiveJitter = Min(Chapter9PositionJitter,
			Min((Chapter9EffectiveSpacing - Chapter9PlatformSize - Chapter9EffectiveGap) * 0.5,
				(Chapter9MaxEdgeLength - Chapter9EffectiveSpacing) * 0.25));

		// 正方形ブロック表面間の最短距離。斜めも実際の隙間で判定する。
		bool HasChapter9PlatformGap(const Vec3& a, const Vec3& b) {
			const double dx = Abs(a.x - b.x), dz = Abs(a.z - b.z);
			if (dx < Chapter9PlatformSize && dz < Chapter9PlatformSize) return false;
			const double gapX = Max(0.0, dx - Chapter9PlatformSize);
			const double gapZ = Max(0.0, dz - Chapter9PlatformSize);
			return gapX * gapX + gapZ * gapZ + 1e-9 >= Chapter9EffectiveGap * Chapter9EffectiveGap;
		}

		bool IsChapter9EdgeReachable(const Vec3& a, const Vec3& b) {
			const double dx = Abs(a.x - b.x), dz = Abs(a.z - b.z);
			const double distance = Math::Sqrt(dx * dx + dz * dz);
			if (distance <= 0.0) return false;
			// 斜め方向ではブロックの投影幅が増える分だけ中心間距離を許可する。
			// 現在の上限4.5なら斜め45度で約5.25。2段ジャンプの到達距離内。
			const double projectedWidth = Chapter9PlatformSize * (dx + dz) / distance;
			return distance <= Chapter9MaxEdgeLength + projectedWidth - Chapter9PlatformSize;
		}

		bool HasChapter9DiagonalPath(const Array<Vec3>& platforms) {
			for (size_t i = 1; i < platforms.size(); ++i) {
				if (Abs(platforms[i].x - platforms[i - 1].x) >= 1.0
					&& Abs(platforms[i].z - platforms[i - 1].z) >= 1.0) return true;
			}
			return platforms.size() <= 1;
		}

		void GenerateChapter9Candidate(Array<Vec3>& platforms, Array<std::pair<int32, int32>>& edges) {
			Array<int32> route{ 0 };
			Array<bool> used(Chapter9GridColumns * Chapter9GridRows, false);
			used[0] = true;
			int32 budget = Chapter9SearchBudget;
			// 隣接セルをランダムな順で探索し、スタートからの一筆書きを作る。
			const auto search = [&](const auto& self) -> bool {
				if (route.size() == Chapter9BlockCount) return true;
				if (--budget < 0) return false;
				const int32 cell = route.back();
				const int32 x = cell % Chapter9GridColumns, z = cell / Chapter9GridColumns;
				Array<int32> candidates;
				if (x > 0) candidates << (cell - 1);
				if (x + 1 < Chapter9GridColumns) candidates << (cell + 1);
				if (z > 0) candidates << (cell - Chapter9GridColumns);
				if (z + 1 < Chapter9GridRows) candidates << (cell + Chapter9GridColumns);
				for (int32 i = static_cast<int32>(candidates.size()) - 1; i > 0; --i) {
					std::swap(candidates[i], candidates[Random(0, i)]);
				}
				for (const int32 next : candidates) {
					if (used[next]) continue;
					used[next] = true;
					route << next;
					if (self(self)) return true;
					route.pop_back();
					used[next] = false;
					if (budget < 0) break;
				}
				return false;
			};
			if (!search(search)) {
				// 上限に達しても指定数とクリア可能な経路を保証する蛇行配置。
				route.clear();
				for (int32 i = 0; i < Chapter9BlockCount; ++i) {
					const int32 row = i / Chapter9GridColumns, column = i % Chapter9GridColumns;
					route << (row * Chapter9GridColumns + ((row % 2 == 0) ? column : Chapter9GridColumns - 1 - column));
				}
			}
			platforms.clear();
			edges.clear();
			// 大きな隙間でも初期状態から斜めのPathを含めるため、行を交互にずらす。
			const double stagger = Min(Chapter9DiagonalOffset, Chapter9EffectiveSpacing * 0.45);
			const bool singleRow = route.all([](int32 cell) { return cell / Chapter9GridColumns == 0; });
			for (int32 i = 0; i < Chapter9BlockCount; ++i) {
				const int32 column = route[i] % Chapter9GridColumns, row = route[i] / Chapter9GridColumns;
				Vec3 pos = Chapter9StartPosition + Vec3{
					column * Chapter9EffectiveSpacing + (row % 2) * stagger, 0.0,
					row * Chapter9EffectiveSpacing + (singleRow ? (column % 2) * stagger : 0.0) };
				platforms << pos;
				if (i > 0) edges.emplace_back(i - 1, i);
			}
			const Array<Vec3> initialPlatforms = platforms;

			// 格子から離れた自由な配置にする。正解経路の距離と隙間を毎回検査するため、
			// 探索が行き詰まっても最後の有効な配置をそのまま使える。

			for (int32 pass = 0; pass < Chapter9ScatterPasses; ++pass) {
				for (int32 i = 1; i < Chapter9BlockCount; ++i) {
					const Vec3 candidate = platforms[i] + Vec3{
						Random(-Chapter9ScatterStep, Chapter9ScatterStep), 0.0,
						Random(-Chapter9ScatterStep, Chapter9ScatterStep) };
					if (candidate.x < Chapter9StartPosition.x - Chapter9EffectiveJitter
						|| candidate.x > Chapter9StartPosition.x + (Chapter9GridColumns - 1) * Chapter9EffectiveSpacing + Chapter9EffectiveJitter
						|| candidate.z < Chapter9StartPosition.z - Chapter9EffectiveJitter
						|| candidate.z > Chapter9StartPosition.z + (Chapter9GridRows - 1) * Chapter9EffectiveSpacing + stagger + Chapter9EffectiveJitter) continue;
					if (!IsChapter9EdgeReachable(candidate, platforms[i - 1])
						|| (i + 1 < Chapter9BlockCount && !IsChapter9EdgeReachable(candidate, platforms[i + 1]))) continue;
					bool overlaps = false;
					for (int32 j = 0; j < Chapter9BlockCount; ++j) {
						if (i != j && !HasChapter9PlatformGap(candidate, platforms[j])) {
							overlaps = true;
							break;
						}
					}
					if (!overlaps) platforms[i] = candidate;
				}
			}
			// ランダム移動の結果が軸方向だけになった場合は斜めを持つ初期配置を使う。
			if (!HasChapter9DiagonalPath(platforms)) platforms = initialPlatforms;
			// 分岐も格子の隣接関係ではなく実際の距離で選び、斜めの辺を許可する。
			for (int32 a = 0; a < Chapter9BlockCount; ++a) {
				for (int32 b = a + 2; b < Chapter9BlockCount; ++b) {
					if (IsChapter9EdgeReachable(platforms[a], platforms[b])
						&& Random(0.0, 1.0) < Chapter9ExtraEdgeProbability) edges.emplace_back(a, b);
				}
			}
		}

		// 正解は0→1→…→最後。途中の区間を飛ばすと、その区間を回収しても
		// 残りへ戻れない分岐を作る。単なる余分な辺ではなく詰みを構造で保証する。
		bool AddChapter9Trap(const Array<Vec3>& platforms, Array<std::pair<int32, int32>>& edges) {
			const int32 count = static_cast<int32>(platforms.size());
			if (count < 4) return false; // 3頂点以下では正解と詰む分岐を両立できない。
			Array<std::pair<int32, int32>> candidates;
			for (int32 from = 0; from < count - 3; ++from) {
				for (int32 to = from + 2; to < count - 1; ++to) {
					if (IsChapter9EdgeReachable(platforms[from], platforms[to])) {
						candidates.emplace_back(from, to);
					}
				}
			}
			if (candidates.isEmpty()) return false;
			const auto [from, to] = candidates[Random(0, static_cast<int32>(candidates.size()) - 1)];
			Array<std::pair<int32, int32>> trapEdges;
			for (const auto& edge : edges) {
				const auto [a, b] = edge;
				// 飛ばした区間に別の出入口があると救済ルートになるため除く。
				const bool touchesSkipped = (from < a && a < to) || (from < b && b < to);
				if (b == a + 1 || !touchesSkipped) trapEdges << edge;
			}
			if (!trapEdges.any([from, to](const auto& edge) { return edge.first == from && edge.second == to; })) {
				trapEdges.emplace_back(from, to);
			}
			edges = std::move(trapEdges);
			// 0…from→toと進むと、未訪問の区間(from,to)への入口はtoのみ。
			// 区間を辿ればfrom手前で行き止まり、後回しにすれば二度と戻れない。
			// toより先にも未訪問ブロックを残しているため、どちらも全訪問できない。
			return true;
		}

		void GenerateChapter9Layout(Array<Vec3>& platforms, Array<std::pair<int32, int32>>& edges) {
			if constexpr (!Chapter9RequireTrap || Chapter9BlockCount < 4) {
				GenerateChapter9Candidate(platforms, edges);
				return;
			}
			for (int32 attempt = 0; attempt < Max(0, Chapter9TrapLayoutAttempts); ++attempt) {
				GenerateChapter9Candidate(platforms, edges);
				if (AddChapter9Trap(platforms, edges)) return;
			}

			// 試行上限時も正解と詰む分岐を保証する。最初の3個で三角形を作り、
			// 以降は交互に半列ずらした行へ蛇行してつなぐ。スタートは固定。
			platforms.clear();
			edges.clear();
			const double spacing = Chapter9EffectiveSpacing;
			platforms << Chapter9StartPosition;
			platforms << (Chapter9StartPosition + Vec3{ spacing, 0.0, 0.0 });
			for (int32 i = 2; i < Chapter9BlockCount; ++i) {
				const int32 row = 1 + (i - 2) / Chapter9GridColumns;
				const int32 offset = (i - 2) % Chapter9GridColumns;
				const int32 column = (row % 2 == 1) ? offset : Chapter9GridColumns - 1 - offset;
				platforms << (Chapter9StartPosition + Vec3{
					(column + ((row % 2 == 1) ? 0.5 : 0.0)) * spacing, 0.0, row * spacing });
			}
			for (int32 a = 0; a < Chapter9BlockCount; ++a) {
				for (int32 b = a + 1; b < Chapter9BlockCount; ++b) {
					if (b == a + 1 || (IsChapter9EdgeReachable(platforms[a], platforms[b])
						&& Random(0.0, 1.0) < Chapter9ExtraEdgeProbability)) edges.emplace_back(a, b);
				}
			}
			AddChapter9Trap(platforms, edges); // 0→2が常に候補になる。
		}

		// 真下の位置を示す影。足場の端で切り取り、空中には描かない。
		void DrawChapter9PlayerShadow(const Vec3& playerPos, const Array<Vec3>& platforms) {
			if (playerPos.y < Chapter9PlatformTop) return;

			static const Texture shadowTexture{ [] {
				Image image{ 64, 64, Color{ 0, 0, 0, 0 } };
				for (int32 y = 0; y < 64; ++y) {
					for (int32 x = 0; x < 64; ++x) {
						const double distance = Vec2{ (x + 0.5 - 32.0) / 32.0, (y + 0.5 - 32.0) / 32.0 }.length();
						image[y][x] = Color{ 0, 0, 0, static_cast<uint8>(255.0 * Clamp((1.0 - distance) / 0.3, 0.0, 1.0)) };
					}
				}
				return image;
			}() };
			static DynamicMesh shadowMesh{ 4, 2 };
			const ScopedRenderStates3D states{
				BlendState::Default2D, RasterizerState::SolidCullNone,
				DepthStencilState::DepthTest, SamplerState::ClampLinear
			};
			const double radius = Chapter9PlayerShadowRadius;
			const double halfSize = Chapter9PlatformSize * 0.5;
			for (const Vec3& platform : platforms) {
				const double left = Max(playerPos.x - radius, platform.x - halfSize);
				const double right = Min(playerPos.x + radius, platform.x + halfSize);
				const double front = Max(playerPos.z - radius, platform.z - halfSize);
				const double back = Min(playerPos.z + radius, platform.z + halfSize);
				if (right <= left || back <= front) continue;

				const auto vertex = [&](double x, double z) -> Vertex3D {
					return {
						Float3{ static_cast<float>(x), static_cast<float>(Chapter9PlatformTop + 0.015), static_cast<float>(z) },
						Float3{ 0.0f, 1.0f, 0.0f },
						Float2{ static_cast<float>((x - playerPos.x + radius) / (2.0 * radius)),
							static_cast<float>((z - playerPos.z + radius) / (2.0 * radius)) }
					};
				};
				shadowMesh.fill(MeshData{
					Array<Vertex3D>{ vertex(left, front), vertex(right, front), vertex(right, back), vertex(left, back) },
					Array<TriangleIndex32>{ { 0, 1, 2 }, { 0, 2, 3 } }
				});
				shadowMesh.draw(shadowTexture, ColorF{ 1.0, Chapter9PlayerShadowOpacity });
				// 次の足場用に頂点を書き換える前に描画を確定する。
				Graphics3D::Flush();
			}
		}

		// 現在の頂点と着地先の足場が辺で接続されているか判定する。
		bool AreChapter9NodesConnected(const Array<std::pair<int32, int32>>& edges, int32 a, int32 b) {
			return edges.any([a, b](const auto& edge) {
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
	void AvoidanceManager::resetChapter9HamiltonPath(bool regenerateLayout) {
		if (regenerateLayout || chapter9Platforms.isEmpty()) {
			GenerateChapter9Layout(chapter9Platforms, chapter9Edges);
		}
		chapter9VisitedNodes.assign(chapter9Platforms.size(), false);
		chapter9PlatformStates.assign(chapter9Platforms.size(), 0);
		chapter9VisitedNodes[0] = true;
		chapter9PlatformStates[0] = 1;
		chapter9PlayerPos = chapter9Platforms[0] + Vec3{ 0.0, Chapter9PlatformTop, 0.0 };
		chapter9PlayerVelocity = Vec3{ 0.0, 0.0, 0.0 };
		chapter9CurrentNode = 0;
		chapter9GroundedPlatform = 0;
		chapter9CanDoubleJump = true;
		chapter9PlayerDirection = Global::Direction::RIGHT;
		chapter9Completed = (chapter9Platforms.size() == 1);
	}

	// Chapter 9の入力、移動、着地、針化、時間切れを毎STEP更新する。
	void AvoidanceManager::chapter9(const ReplayInputFrame& input) {
		if (chapter9Platforms.isEmpty() || chapter9VisitedNodes.size() != chapter9Platforms.size()) {
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
			const Vec3 platformPos = chapter9Platforms[chapter9GroundedPlatform];
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
			for (int32 index = 0; index < static_cast<int32>(chapter9Platforms.size()); ++index) {
				if (chapter9PlatformStates[index] == 2
					|| chapter9VisitedNodes[index]
					|| !AreChapter9NodesConnected(chapter9Edges, chapter9CurrentNode, index)) {
					continue;
				}
				const Vec3 platformPos = chapter9Platforms[index];
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
		if (activeChapter != 9 || chapter9Platforms.isEmpty() || chapter9VisitedNodes.size() != chapter9Platforms.size()) {
			return;
		}

		// 深度バッファ付きの描画先を使い、3Dオブジェクトの前後関係を正しく判定する。
		static const MSRenderTexture renderTexture{
			Size{ Global::windowWidth, Global::windowHeight },
			TextureFormat::R8G8B8A8_Unorm_SRGB,
			HasDepth::Yes
		};
		renderTexture.clear(backgroundColor);

		// STEP基準で位相を進め、開始時は左端（速度0）、中央で速度最大になる。
		const double cameraTimeSeconds = step / static_cast<double>(Global::FPS);
		const double cameraX = Chapter9CameraAmplitude * Math::Sin(
			Math::TwoPi * cameraTimeSeconds / Chapter9CameraPeriodSeconds - Math::HalfPi);
		// 行ずらしや大きな隙間を含む、生成された配置全体にカメラを合わせる。
		double minX = chapter9Platforms[0].x, maxX = minX;
		double minZ = chapter9Platforms[0].z, maxZ = minZ;
		for (const Vec3& pos : chapter9Platforms) {
			minX = Min(minX, pos.x); maxX = Max(maxX, pos.x);
			minZ = Min(minZ, pos.z); maxZ = Max(maxZ, pos.z);
		}
		const double cameraScale = Max(1.0, Max((maxX - minX) / 14.0, (maxZ - minZ) / 7.0));
		const Vec3 cameraTarget{ (minX + maxX) * 0.5, 0.0, (minZ + maxZ) * 0.5 + 0.5 };
		const BasicCamera3D camera{
			renderTexture.size(), 35_deg,
			cameraTarget + Vec3{ cameraX, 18.0 * cameraScale, -19.0 * cameraScale }, cameraTarget
		};
		int32 visitedCount = 0;
		{
			const ScopedRenderTarget3D target{ renderTexture };
			// Hamilton Path全体を見渡せるようにカメラと照明を設定する。
			Graphics3D::SetCameraTransform(camera);
			Graphics3D::SetGlobalAmbientColor(ColorF{ 0.72 });
			Graphics3D::SetSunDirection(Vec3{ 0.4, 1.0, -0.3 }.normalized());

		// グラフの辺を3D空間上の線として描画する。
		for (const auto& [from, to] : chapter9Edges) {
			Line3D{
				chapter9Platforms[from] + Vec3{ 0.0, Chapter9PlatformTop + 0.03, 0.0 },
				chapter9Platforms[to] + Vec3{ 0.0, Chapter9PlatformTop + 0.03, 0.0 }
			}.draw(ColorF{ 0.25, 0.70, 0.90, 0.85 });
		}

		// 各頂点をブロックで描画し、離れた足場には四角錐の針を重ねる。
		for (int32 index = 0; index < static_cast<int32>(chapter9Platforms.size()); ++index) {
			if (chapter9VisitedNodes[index]) ++visitedCount;
			const Vec3 platformPos = chapter9Platforms[index];
			const ColorF platformColor = chapter9VisitedNodes[index]
				? ColorF{ 0.62, 1.0, 0.78 } : ColorF{ 0.90 };
			Box{ platformPos, Chapter9PlatformSize, Chapter9PlatformHeight, Chapter9PlatformSize }
				.draw(TextureAsset(U"sprBlock"), platformColor);
			if (chapter9PlatformStates[index] == 2) {
				GetChapter9SpikeMesh().draw(
					platformPos + Vec3{ 0.0, Chapter9PlatformTop, 0.0 }, TextureAsset(U"sprSpike"));
			}
		}

		DrawChapter9PlayerShadow(chapter9PlayerPos, chapter9Platforms);

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
		FontAsset(U"Button")(Format(visitedCount) + U" / " + Format(chapter9Platforms.size()))
			.draw(Vec2{ 24, 48 }, ColorF{ 0.92 });
		FontAsset(U"Button")(U"TIME " + ToFixed(remainingStep / static_cast<double>(Global::FPS), 2))
			.draw(Vec2{ 640, 20 }, timerColor);
		if (chapter9Completed) {
			FontAsset(U"Title")(U"PATH COMPLETE").drawAt(Vec2{ 400, 76 }, ColorF{ 0.30, 1.0, 0.72 });
		}
	}
}
