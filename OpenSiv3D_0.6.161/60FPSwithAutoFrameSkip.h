#pragma once
# include <Siv3D.hpp> //OpenSiv3D v0.6.16 動作チェック済み
namespace chrono = std::chrono;

#pragma region このファイルの使い方(設定方法)
/*
このヘッダーファイルの機能

画素数を１２８０×７２０にして、家庭用ゲームと同じフレームレートにする（６０）

使い方：
このファイルをプロジェクトに入れ、

下記ファイルにに１行追加して下さい ＜● は3箇所あるので注意＞

Main.cpp の3箇所を変更する方法

----- stdafx.h -----
# pragma once
//# define NO_S3D_USING
# include <Siv3D.hpp>
#include"60FPSwithAutoFrameSkip.h" // ● <<<<<<<< １行追加

そうしたら、
System::Update();  を System60::Update();
に変更して下さい


Main関数に２点を追加と変更して下さい

  例：
	void Main()
	{
											　　//好きな画面サイズにして
		System60::SetDisplaySize(DisplayResolution::HD_1280x720);  <<<<● 追加して
		・・・
		・・・
		・・・
		while(System60::Update())   <<<<● 60を追加して
		{
*/
/// 0:
/// @brief このファイル 60FPSwithAutoFrameSkip.h をMain.cpp と同じ場所にコピーしてください
/// @brief そうしたら、Main.cpp か stdafx.h どちらかの
/// @brief #include <Siv3D.hpp> の下の行に
/// @brief #include"60FPSwithAutoFrameSkip.h" を追加してください 
constexpr int s60_tutorial_0 = 0;
/// 1:
/// @brief main(){のすぐ下に
/// @brief System60::SetDisplaySize(DisplayResolution::～) で設定して下さい
constexpr int s60_tutorial_1 = 1;
/// 2:
/// @brief System::Update()  を全て>>
/// @brief System60::Update() に変更して下さい
constexpr int s60_tutorial_2 = 2;
/// 3:
/// @brief tutorial_0～2 ができればOKです
constexpr int s60_tutorial_3 = 3;

#pragma endregion

/*
※ # include <Siv3D.hpp> の下の行に
#include"60FPSwithAutoFrameSkip.h" が入っており、
さらにそのファイルがMain.cppと同じ場所にある時、

～.cpp ファイル内で s60_ と打ち込むと説明メニューが出ます。

*/

/// @brief System60::SetDisplaySize(～ で設定して、System::Update()  >> System60::Update() に変更して下さい
class System60
{
private:
	/// @brief インスタンス生成できません
	System60() = default;
public:

	inline static chrono::high_resolution_clock::time_point start; // 計測スタート時刻を保存 //XXXX

	static const int FPS = 50; // 1秒間に1画面を書き換える回数
	/// @brief フルスクリーンの画素数
	inline static Size displayResolution = DisplayResolution::HD_1280x720;
	inline static bool IsFullScreen = false;

	/// @brief 画面サイズを設定する
	/// @param displayResolution DisplayResolution::～～ で設定
	/// @param IsFullScreen trueでフルスクリーン、 リリース時以外は基本はウィンドウで開発したほうがいいです
	inline static void SetDisplaySize(Size _displayResolution, bool _IsFullScreen = false)
	{
		System60::displayResolution = _displayResolution;
		System60::IsFullScreen = _IsFullScreen;
		DisplaySet();
	}

#pragma optimize("", off)
	/// @brief ６０FPS　で System::Update() を行う 
	/// @return フレームスキップする場合、描画しない
	static bool Update()
	{
		_ASSERT_EXPR(IsDisplaySet, L"\n\n  while(System60::Update())  の前に \n\n System60::SetDisplaySize(～ )\n\nで設定して下さい ");

		if (displayResolution.x == 0)
		{
			displayResolution = DisplayResolution::HD_1280x720;
			DisplaySet();	// 本当はMain()に入る直前に行いたいのだが仕方なく
		}

		auto elapset = [&]()
			{
				auto end = chrono::high_resolution_clock::now();
				auto durtion = end - start;        // 要した時間を計算
				auto nanosecound = chrono::duration_cast<std::chrono::nanoseconds>(durtion).count();
				return nanosecound;
			};
		/// @brief フレームスキップ確認、１フレーム時間を超えたら描画せずに次のループへ
		/// @return true
		if (elapset() > 1000 * 1000 * 1000 / (long long)FPS)
		{
			start = chrono::high_resolution_clock::now();      // 計測スタート時刻を保存
			return true;
		}
		long long sleepTime = elapset() / (long long)(1000000.0 - 100.0) / (long long)2.0; // -100.0 は微調整
		System::Sleep((int32)sleepTime);
		while (elapset() < 1000 * 1000 * 1000 / FPS); //16
		// Sleep精度が不安なのでしかたなく待機時間の半分を空ループにした。動作は安定している

		start = chrono::high_resolution_clock::now();      // 計測スタート時刻を保存

		return System::Update();
	}

private:
	inline static bool IsDisplaySet = false;
	/// @brief 画面サイズ変更
	/// @param displayResolution 画素数指定 
	/// @return 画面変更成功
	static bool DisplaySet()
	{
		IsDisplaySet = true;
		Scene::SetResizeMode(ResizeMode::Keep);
		bool ok = Window::Resize(displayResolution);
		Scene::Resize(displayResolution);
		if (IsFullScreen)
		{
			Window::SetFullscreen(true);
		}
		return ok;
	}
	/// @brief 画面の画素数を設定する
	/// 基本的には、引数は一つだけにするが、
	/// Windowサイズとシーンサイズの画素数が違う場合、２つ引数を書く
	/// @param windowsSize ウィンドウの実画素数 ex  DisplayResolution::HD_1280x720
	/// @param sceneSize ウィンドウ内の仮想の画素数  ここは書かなくても可能
	/// @return ウィンドウサイズの変更に成功したら true
	static bool DisplaySet(Size windowsSize, Size sceneSize = { 0,0 })
	{
		displayResolution = windowsSize;
		bool ok = Window::Resize(windowsSize);
		if (sceneSize.x != 0)
		{
			Scene::Resize(windowsSize);
		}
		return ok;
	}
public:
	/// @brief  指定したミリ秒待つ
	/// @param millisecond ミリ秒
	static void Sleep(int32 millisecond)
	{
		System::Sleep(millisecond);
	}
	/// @brief 指定した秒数待つ
	/// @param duration 秒数（少数可能）
	static void Sleep(Duration const& duration)
	{
		System::Sleep(duration);
	}
	/// <summary>アプリケーションの終了</summary>
	static void Exit()
	{
		System::Exit();
	}

	//if (System::MessageBoxOKCancel(mes)
	//
	static MessageBoxResult MessageBoxOKCancel(const String& mes)
	{
		return System::MessageBoxOKCancel(mes);
	}
	/// @brief 画面表示前の初期化  諸事情でpublicですが、使わないで下さい。動作がおかしくなります。
	/// @return 
	static bool Initialize()
	{
		start = chrono::high_resolution_clock::now();      // 計測スタート時刻を保存
		//displayResolution = { 0,0 };

		//後で使うかもしれないので保存  // NOLINT(cpp:S1135)
		//auto end = chrono::high_resolution_clock::now();
		//auto durtion = end - start;        // 要した時間を計算
		//auto nsec = duration_cast<std::chrono::nanoseconds>(durtion).count();  未使用

		return true;	// ここでは最適化無視しないと怖い
	}
};
const bool bootOnlySysytem_0 = { System60::Initialize() };


