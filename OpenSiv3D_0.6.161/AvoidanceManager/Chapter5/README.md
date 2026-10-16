# Chapter5 サンプル

Chapter5を選んで開始すると、放射弾、自機狙いの扇弾、波打つ落下弾が順番に出現します。既存の足場とプレイヤー設定を使用しています。

## 編集する場所

| 変更したいもの | ファイル |
| --- | --- |
| 発射タイミング・パターンの組み合わせ | `Chapter5Script.cpp` |
| 弾数・速度・色などの既定値 | `Chapter5Patterns.h` |
| 弾の配置・発射方向の計算 | `Chapter5Patterns.cpp` |
| Cherry単体の移動 | `../../GameObject/Cherries/Chapter5/Chapter5CherryBehaviors.cpp` |

`Chapter5Manager.cpp` は既存マネージャーとの接続だけを行います。生成には既存の `createCherry` を使い、更新・描画・当たり判定・画面外削除・プールへの回収は既存処理が担当します。

## 時間の単位

`Chapter5Script` の時間はChapter5開始を0とする曲のstepです。開始位置には `Global::startStep_Chapter5` を使用します。

- `at(20, ...)`: 相対step 20を通過したときに一度発射。
- `every(24, 44, 140, ...)`: 相対step 44〜140の間、24stepごとに発射。
- `every` の `localStep`: その区間の開始からの経過step。
- `Cherry::Behavior` の `age`: 生成後の更新フレーム数。曲のstepとは異なります。

弾の移動速度は1更新フレームあたりの距離、角度は度数法（右0度、上90度）です。各弾は画面外、または寿命360フレームで削除されます。自機狙いは発射時のプレイヤー位置を狙います。

## パターンを追加するとき

1. 新しい移動が必要なら `Chapter5CherryBehaviors` に `Cherry::Behavior` を返す関数を追加します。
2. `Chapter5Patterns` に設定構造体と生成関数を追加します。
3. `Chapter5Script.cpp` の進行表から呼び出します。

サンプルの進行は状態を持たず、リトライ時のリセット処理は不要です。将来、演出や複数弾で共有する状態を `Chapter5Script` に持たせる場合は、チャプター開始・リトライ時の初期化も追加してください。Cherryの挙動にはマネージャーへの参照を保持させず、必要な値をコピーして渡す構成です。
