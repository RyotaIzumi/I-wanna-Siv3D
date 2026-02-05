# pragma once
# include <Siv3d.hpp>

namespace Iwanna {
	namespace Sound {

		const String MAIN{ U"sndHibana" };
		const FilePath Path_MAIN{ U"Sound/Bgm/sndHibana.mp3" };

		//SEの名前とパスの定数
		static const String SE_PATH = U"Sound/Se/";

		static const String DEATH = U"sndDeath";
		static const String DEATH_PATH = SE_PATH + DEATH + U".wav";
		static const String JUMP = U"sndJump";
		static const String JUMP_PATH = SE_PATH + JUMP + U".wav";
		static const String DJUMP = U"sndDJump";
		static const String DJUMP_PATH = SE_PATH + DJUMP + U".wav";
		static const String SHOOT = U"sndShoot";
		static const String SHOOT_PATH = SE_PATH + SHOOT + U".wav";
		
		void registerBGMs();
		void registerSEs();
		void registerAudio(const String& name, const String& path, const Loop loop = Loop::No);
		void registerAudiosSync();
		void loadAudiosSync();
	}
}
