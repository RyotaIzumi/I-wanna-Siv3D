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

	bool readBool(const std::string& json, const char* key, bool& value) {
		const std::regex pattern(std::string("\"") + key + R"("\s*:\s*(true|false))");
		std::smatch match;
		if (!std::regex_search(json, match, pattern)) {
			return false;
		}

		value = (match[1].str() == "true");
		return true;
	}

	void readIntArray(const std::string& json, const char* key, std::array<int32, Iwanna::SaveData::ChapterCount>& values) {
		const std::regex pattern(std::string("\"") + key + R"("\s*:\s*\[([^\]]*)\])");
		std::smatch match;
		if (!std::regex_search(json, match, pattern)) {
			return;
		}

		const std::string body = match[1].str();
		const std::regex numberPattern(R"(-?\d+)");
		auto it = std::sregex_iterator(body.begin(), body.end(), numberPattern);
		const auto end = std::sregex_iterator{};

		for (size_t index = 0; index < values.size() && it != end; ++index, ++it) {
			values[index] = std::max(0, std::stoi((*it).str()));
		}
	}

	void readStringArray(const std::string& json, const char* key, std::array<String, Iwanna::SaveData::AchievementCount>& values) {
		const std::regex pattern(std::string("\"") + key + R"("\s*:\s*\[([^\]]*)\])");
		std::smatch match;
		if (!std::regex_search(json, match, pattern)) {
			return;
		}

		const std::string body = match[1].str();
		const std::regex stringPattern("\"([^\"]*)\"");
		auto it = std::sregex_iterator(body.begin(), body.end(), stringPattern);
		const auto end = std::sregex_iterator{};
		Array<String> parsed;

		for (; it != end; ++it) {
			parsed << Unicode::FromUTF8((*it)[1].str());
		}

		if (parsed.size() == Iwanna::SaveData::ChapterCount
			&& values.size() == Iwanna::SaveData::AchievementCount) {
			for (size_t index = 0; index < parsed.size(); ++index) {
				values[index + 1] = parsed[index];
			}
			return;
		}

		for (size_t index = 0; index < values.size() && index < parsed.size(); ++index) {
			values[index] = parsed[index];
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
		if (readNumber(json, "bgmVolume", value)) {
			bgmVolume = std::clamp(value, 0.0, 1.0);
		}
		if (readNumber(json, "seVolume", value)) {
			seVolume = std::clamp(value, 0.0, 1.0);
		}
		if (readNumber(json, "difficulty", value)) {
			difficulty = static_cast<Global::Difficulty>(std::clamp(static_cast<int32>(value), 0, 2));
		}
		readBool(json, "hasStartedAvoidance", hasStartedAvoidance);
		readIntArray(json, "chapterDeathCounts", chapterDeathCounts);
		readStringArray(json, "achievementUnlockedAt", achievementUnlockedAt);
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
			<< roundToMillis(highestEnduranceSec) << ",\n";
		writer << "  \"bgmVolume\": " << std::defaultfloat << bgmVolume << ",\n";
		writer << "  \"seVolume\": " << seVolume << ",\n";
		writer << "  \"difficulty\": " << static_cast<int32>(difficulty) << ",\n";
		writer << "  \"hasStartedAvoidance\": " << (hasStartedAvoidance ? "true" : "false") << ",\n";
		writer << "  \"chapterDeathCounts\": [";
		for (size_t i = 0; i < chapterDeathCounts.size(); ++i) {
			if (i != 0) writer << ", ";
			writer << chapterDeathCounts[i];
		}
		writer << "],\n";
		writer << "  \"achievementUnlockedAt\": [";
		for (size_t i = 0; i < achievementUnlockedAt.size(); ++i) {
			if (i != 0) writer << ", ";
			writer << "\"" << Unicode::ToUTF8(achievementUnlockedAt[i]) << "\"";
		}
		writer << "]\n";
		writer << "}\n";
	}

	void SaveData::addDeath(int32 chapter) {
		++deathCount;
		if (1 <= chapter && chapter <= ChapterCount) {
			++chapterDeathCounts[chapter - 1];
		}
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

	bool SaveData::unlockAchievement(int32 index) {
		if (index < 0 || AchievementCount <= index || !achievementUnlockedAt[index].isEmpty()) {
			return false;
		}

		achievementUnlockedAt[index] = DateTime::Now().format(U"yyyy-MM-dd HH:mm:ss");
		return true;
	}
}
