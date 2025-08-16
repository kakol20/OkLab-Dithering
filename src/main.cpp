#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "../ext/json/json.hpp"
using json = nlohmann::json;

#include "image/Image.h"

/*
	TODO:
		Replace iostream with Log class
*/

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	std::ifstream settingsLoc("data/settings.json");
	if (!(settingsLoc)) {
		std::cout << "JSON not found\n";
		std::cin.ignore();

		return -1;
	}
	json settings = json::parse(settingsLoc);

	std::string imageLoc = "data/suzanne.png";
	Image::ImageType imageType = Image::GetFileType("data/suzanne.png");
	if (imageType == Image::ImageType::NA) {
		std::cout << "Image not found\n";
		std::cin.ignore();

		return -1;
	}
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

	std::cout << "Press a key to exit...\n";
	std::cin.ignore();
	return 0;
}