#pragma once
#include <Siv3D.hpp>
#include "../CommonData.h"
#include "../Global.h"

namespace Iwanna {
	static const Duration TRANSITION_DURATION = 0.50s;

	enum class SceneType {
		LOADING,
		START_MENU,
		IN_GAME
	};

	using App = SceneManager<SceneType,CommonData>;

	class Loading : public App::Scene {
	private:
		//Profiler profiler;
		AsyncTask<void> loadingTask;

	public:
		
		Loading(const InitData& init);
		void load();
		void update() override;
		void draw() const override;
	};

	class Title : public App::Scene {
	private:
	public:
		/*
		Title(const InitData& data);

		void update() override;
		void draw() const override;
		*/
	};

	class StartMenu : public App::Scene {
	private:
	public:
		StartMenu(const InitData& data);

		void update() override;
		void draw() const override;
	};

	class InGame : public App::Scene {
	private:
	public:
		InGame(const InitData& data);

		void update() override;
		void debug() const;
		void draw() const override;
	};
}
