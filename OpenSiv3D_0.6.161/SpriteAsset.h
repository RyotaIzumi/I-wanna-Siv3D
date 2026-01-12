#pragma once
#include<Siv3D.hpp>

namespace Iwanna {
	void registerTextures();
	void registerTexture(const String& name, const String& path);
	void registerTexturesSync();
	void loadTexturesSync();
}
