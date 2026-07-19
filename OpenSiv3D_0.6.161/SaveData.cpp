#include "SaveData.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <string>

namespace {
	constexpr const char* SaveFilePath = "save.json";

	bool readFile(const char* path, std::string& text) {
		std::ifstream reader(path);
		if (!reader) {
			return false;
		}

		text.assign(
			std::istreambuf_iterator<char>(reader),
			std::istreambuf_iterator<char>());
		return true;
	}

	bool readNumber(const std::string& json, const char* key, double& value) {
		const std::regex pattern(
			std::string("\"") + key + R"("\s*:\s*(-?\d+(?:\.\d+)?))");
		std::smatch match;
		if (!std::regex_search(json, match, pattern)) {
			return false;
		}

		try {
			value = std::stod(match[1].str());
			return true;
		}
		catch (...) {
			return false;
		}
	}

	double roundToMillis(double value) {
		return Math::Round(value * 1000.0) / 1000.0;
	}
}

namespace Iwanna {
	void SaveData::load() {
		std::string json;
		if (!readFile(SaveFilePath, json)) {
			return;
		}

		double value = 0.0;
		if (readNumber(json, "deathCount", value)) {
			deathCount = std::max(0, static_cast<int32>(value));
		}
		if (readNumber(json, "playTimeSec", value)) {
			playTimeSec = std::max(0.0, value);
		}
		if (readNumber(json, "highestChapter", value)) {
			highestChapter = std::clamp(static_cast<int32>(value), 1, 6);
		}
		if (readNumber(json, "highestEnduranceSec", value)) {
			highestEnduranceSec = std::max(0.0, roundToMillis(value));
		}
	}

	void SaveData::save() const {
		std::ofstream writer(SaveFilePath, std::ios::trunc);
		if (!writer) {
			return;
		}

		writer << "{\n";
		writer << "  \"deathCount\": " << deathCount << ",\n";
		writer << "  \"playTimeSec\": " << playTimeSec << ",\n";
		writer << "  \"highestChapter\": " << highestChapter << ",\n";
		writer << "  \"highestEnduranceSec\": " << std::fixed << std::setprecision(3)
			<< roundToMillis(highestEnduranceSec) << "\n";
		writer << "}\n";
	}

	void SaveData::addDeath() {
		++deathCount;
	}

	void SaveData::addPlayTime(double seconds) {
		playTimeSec += std::max(0.0, seconds);
	}

	void SaveData::updateHighestChapter(int32 chapter) {
		highestChapter = Max(highestChapter, Clamp(chapter, 1, 6));
	}

	void SaveData::updateHighestEnduranceSec(double seconds) {
		highestEnduranceSec = Max(highestEnduranceSec, roundToMillis(Max(0.0, seconds)));
	}
}
