#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "../ext/json/json.hpp"
using json = nlohmann::json;

/*
	TODO:
		Replace iostream with Log class
*/

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	std::ifstream f("data/settings.json");

	if (!(f)) {
		std::cout << "JSON not found\n";
		std::cin.ignore();

		return -1;
	}

	json settings = json::parse(f);
#else
	// TODO: add
#endif // _DEBUG

	// ========== GET SETTINGS ==========

	std::cout << "===== GETTING SETINGS =====\n";
	std::array<std::string, 4> required = {
		"dither",
		"grayscale",
		"dist_lightness",
		"ordered_dither"
	};
	bool allFound = true;
	for (const auto& key : required) {
		if (!settings.contains(key)) {
			std::cout << "JSON setting not found: " << key << '\n';
			allFound = false;
		} else {
			std::cout << key << ": " << (bool)settings[key] << '\n';
		}
	}

	if (!allFound) {
		return -1;
	}

	std::cin.ignore();
	return 0;
}