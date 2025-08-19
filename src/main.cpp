#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "../ext/json/json.hpp"
using json = nlohmann::json;

#include "image/Colour.h"
#include "image/Dither.h"
#include "image/Image.h"
#include "image/Palette.h"
#include "wrapper/Log.h"
#include "wrapper/Maths.hpp"

//const double Maths::Pi = 3.1415926535;
//const double Maths::Tau = 6.283185307;
//const double Maths::RadToDeg = 180. / Maths::Pi;
//const double Maths::DegToRad = Maths::Pi / 180.;

std::string Extension(const std::string loc);
std::string NoExtension(const std::string loc);

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	std::ifstream settingsLoc("data/settings.json");
	if (!(settingsLoc)) {
		Log::WriteOneLine("JSON not found");
		Log::Save();
		Log::HoldConsole();
		return -1;
	}
	json settings = json::parse(settingsLoc);

	std::string imageLoc = "data/test.png";
	Image::ImageType imageType = Image::GetFileType(imageLoc.c_str());
	if (imageType == Image::ImageType::NA) {
		Log::WriteOneLine("Image not found");
		Log::Save();
		Log::HoldConsole();
		return -1;
	}

	std::string paletteLocStr = "data/minecraft_map_sc.palette";
	std::ifstream paletteLoc(paletteLocStr);
	if (!(paletteLoc)) {
		Log::WriteOneLine("Palette not found");
		Log::Save();
		Log::HoldConsole();
		return -1;
	}
#else
	if (argc < 4) {
		Log::WriteOneLine("Drag and drop an image file, a .palette file and a .json file");
		Log::WriteOneLine("Note: Only PNG, JPG, BMP or TGA image files are supported");

		Log::Save();
		Log::HoldConsole();
		return -1;
	}

	json settings;
	std::string paletteLocStr;
	std::string imageLoc;
	for (int i = 0; i < argc; i++) {
		Log::WriteOneLine(argv[i]);
		std::string extension = Extension(argv[i]);

		if (extension == ".exe") {
			continue;
		} else if (extension == ".json") {
			std::ifstream settingsLoc(argv[i]);
			if (!(settingsLoc)) {
				Log::WriteOneLine("JSON not found");
				Log::Save();
				Log::HoldConsole();
				return -1;
			}
			settings = json::parse(settingsLoc);
		} else if (extension == ".palette") {
			paletteLocStr = argv[i];
			std::ifstream paletteLoc(paletteLocStr);
			if (!(paletteLoc)) {
				Log::WriteOneLine("Palette not found");
				Log::Save();
				Log::HoldConsole();
				return -1;
			}
		} else {
			imageLoc = argv[i];
			Image::ImageType imageType = Image::GetFileType(imageLoc.c_str());
			if (imageType == Image::ImageType::NA) {
				Log::WriteOneLine("Image not found");
				Log::Save();
				Log::HoldConsole();
				return -1;
			}
		}
	}
	Log::EndLine();
#endif // _DEBUG

	// ========== GET SETTINGS ==========

	Log::WriteOneLine("===== GETTING SETINGS =====");
	std::unordered_map<std::string, json::value_t> required = {
		{ "grayscale", json::value_t::boolean },
		{ "dist_lightness", json::value_t::boolean },
		{ "ditherType", json::value_t::string },
		{ "distanceMode", json::value_t::string },
		{ "mathMode", json::value_t::string },
	};
	bool allFound = true;
	for (auto it = required.begin(); it != required.end(); it++) {
		if (!settings.contains(it->first)) {
			Log::WriteOneLine("JSON setting not found: " + it->first);
			allFound = false;
		} else if (settings[it->first].type() != it->second) {
			Log::WriteOneLine("Wrong value type: " + it->first);
			allFound = false;
		} else if (it->second == json::value_t::boolean) {
			Log::WriteOneLine(it->first + ": " + Log::ToString((bool)settings[it->first]));
		} else if (it->second == json::value_t::string) {
			std::string value = settings[it->first];
			std::transform(value.begin(), value.end(), value.begin(), ::tolower);
			settings[it->first] = value;
			Log::WriteOneLine(it->first + ": \"" + (std::string)settings[it->first] + "\"");
		}
	}

	if (!allFound) {
		Log::Save();
		Log::HoldConsole();
		return -1;
	}
	bool invalidType = false;
	if (settings["ditherType"] == "floyd" || settings["ditherType"] == "floyd-steinberg" ||
		settings["ditherType"] == "steinberg" || settings["ditherType"] == "fs") {
		settings["ditherType"] = "fs";
	} else if (settings["ditherType"] == "ordered" || settings["ditherType"] == "bayer") {
		settings["ditherType"] = "ordered";
	} else if (settings["ditherType"] == "none") {
		settings["ditherType"] = "none";
	} else {
		Log::WriteOneLine("Invalid ditherType: " + settings["ditherType"]);
		invalidType = true;
	}

	if (settings["distanceMode"] != "srgb" && settings["distanceMode"] != "oklab") {
		Log::WriteOneLine("Invalid distanceMode: " + settings["distanceMode"]);
		invalidType = true;
	}

	if (settings["mathMode"] != "srgb" && settings["mathMode"] != "oklab") {
		Log::WriteOneLine("Invalid mathMode: " + settings["distanceMode"]);
		invalidType = true;
	}

	if (invalidType) {
		Log::Save();
		Log::HoldConsole();
		return -1;
	}

	// ========== GET IMAGE ==========

	Log::EndLine();
	Log::WriteOneLine("===== GETTING IMAGE =====");

	Image image;
	if (!image.Read(imageLoc.c_str())) {
		Log::Save();
		Log::HoldConsole();
		return -1;
	}

	// ========== GET PALETTE ==========

	Log::EndLine();
	Log::WriteOneLine("===== GETTING PALETTE =====");
	Palette palette(paletteLocStr.c_str());

	// ========== DITHERING ==========

	Log::EndLine();
	Log::WriteOneLine("===== DITHERING =====");

	if (settings["ditherType"] == "ordered") {
		Dither::OrderedDither(image, palette, settings["distanceMode"], settings["mathMode"]);
	} else if (settings["ditherType"] == "fs") {
		Dither::FloydDither(image, palette, settings["distanceMode"], settings["mathMode"]);
	} else {
		Dither::NoDither(image, palette, settings["distanceMode"]);
	}


	// ===== Generate Output Path =====

	const std::string folder = NoExtension(imageLoc);
	std::filesystem::create_directories(folder);

	std::string outputLoc;

	if (settings["ditherType"] == "none") {
		outputLoc = folder + "\\" +
			(std::string)settings["ditherType"] + "-" +
			(std::string)settings["distanceMode"] + ".png";
	} else {
		outputLoc = folder + "\\" +
			(std::string)settings["ditherType"] + "-" +
			(std::string)settings["distanceMode"] + "-" +
			(std::string)settings["mathMode"] + ".png";
	}

	image.Write(outputLoc.c_str());

	Log::Save();
	//Log::HoldConsole();
	return 0;
}

std::string Extension(const std::string loc) {
	std::filesystem::path p = loc;
	return p.extension().string();
}

std::string NoExtension(const std::string loc) {
	std::filesystem::path p = loc;
	std::filesystem::path noExt = p.parent_path() / p.stem();
	return noExt.string();
}