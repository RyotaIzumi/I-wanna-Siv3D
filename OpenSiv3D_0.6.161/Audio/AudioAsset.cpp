# include "AudioAsset.h"
# include <cassert>

namespace Iwanna {
	namespace Sound {

		struct SoundRegistry {
			String name;
			String path;
			Loop loop = Loop::No;
		};

		Array<SoundRegistry> registry;

		//BGMを登録する
		void registerBGMs() {
			registerAudio(MAIN, Path_MAIN, Loop::No);
		}
		
		//SEを登録する
		void registerSEs() {
			registerAudio(DEATH, DEATH_PATH);
			registerAudio(JUMP, JUMP_PATH);
			registerAudio(DJUMP, DJUMP_PATH);
			registerAudio(SHOOT, SHOOT_PATH);
		}

		/**
		 * @brief オーディオを登録する
		 * @param name オーディオ名
		 * @param path オーディオファイルのパス
		 * @param loop ループ設定
		 */
		void registerAudio(const String& name, const String& path, const Loop loop) {
			registry << SoundRegistry{ name, path, loop };
		}

		
		 // オーディオを同期的に登録する
		void registerAudiosSync() {
			for (auto& r : registry) {
				AudioAsset::Register(r.name, r.path, r.loop);
			}
		}

		
		// オーディオを同期的にロードする
		void loadAudiosSync() {
			for (auto& r : registry) {
				AudioAsset::Load(r.name);
			}
		}
	}
}
