#include "AvoidanceManager.h"

namespace Iwanna {
	// step :0 〜 839
	void AvoidanceManager::chapter1() {
		//テスト用に毎秒拡散弾生成
		
		int32 spreadNum = step / 50;
		switch (step % 50) {
			case 0:
				createCherrySpread(Vec2{ 400,300 }, 100 + 2*spreadNum, 3.0,[]() { return std::make_shared<Cherry>(); });
				break;
		}
		
	}
}
