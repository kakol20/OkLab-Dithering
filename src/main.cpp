#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

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

	std::string imageLoc = "data/suzanne.png";
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
	// TODO: add
#endif // _DEBUG

	// ========== GET SETTINGS ==========

	Log::WriteOneLine("===== GETTING SETINGS =====");
	std::map<std::string, json::value_t> required = {
		{ "grayscale", json::value_t::boolean },
		{ "dist_lightness", json::value_t::boolean },
		{ "ditherType", json::value_t::string },
		{ "distanceMode", json::value_t::string },
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

	Dither::OrderedDither(image, palette, (std::string)settings["distanceMode"]);

	// ===== Generate Output Path =====

	const std::string outputLoc = NoExtension(imageLoc) + "_" + 
		(std::string)settings["ditherType"] + "-" + 
		(std::string)settings["distanceMode"] + ".png";

	image.Write(outputLoc.c_str());

	Log::Save();
	Log::HoldConsole();
	return 0;
}

std::string NoExtension(const std::string loc) {
	std::filesystem::path p = loc;
	std::filesystem::path noExt = p.parent_path() / p.stem();
	return noExt.string();
}