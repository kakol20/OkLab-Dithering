#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "image/Dither.h"
#include "image/Image.h"
#include "image/Palette.h"
#include "wrapper/Log.h"

// https://json.nlohmann.me/home/exceptions/#switch-off-exceptions
#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception) { Log::WriteOneLine((exception).what()); }\

#include "../ext/json/json.hpp"
#include <cctype>
#include <cstdlib>
using json = nlohmann::json;

//const double Maths::Pi = 3.1415926535;
//const double Maths::Tau = 6.283185307;
//const double Maths::RadToDeg = 180. / Maths::Pi;
//const double Maths::DegToRad = Maths::Pi / 180.;

std::string Extension(const std::string loc);
std::string NoExtension(const std::string loc);

bool CheckColourMathMode(const std::string& mode);

int main(int argc, char* argv[]) {
	// For generating blue noise array from blue noise texture
	//#define DEV_MODE
#ifdef DEV_MODE
	// https://github.com/Calinou/free-blue-noise-textures
	Image blueNoise("dev/32_LDR_LLL1_0.png");
	Log::WriteOneLine("Grayscale: " + Log::ToString(blueNoise.IsGrayscale()));

	Log::Write("\n{\n");
	for (int y = 0; y < blueNoise.GetHeight(); ++y) {
		Log::Write("\t");
		for (int x = 0; x < blueNoise.GetWidth(); ++x) {
			const uint8_t val = blueNoise.GetData(blueNoise.GetIndex(x, y));
			const std::string valStr = Log::ToString((unsigned int)val, 3, ' ');
			Log::Write(valStr + ", ");
		}
		Log::EndLine();
	}
	Log::Write("};\n");

#else
#ifdef _DEBUG
	std::ifstream settingsLoc("data/settings.json");
	if (!(settingsLoc)) {
		Log::WriteOneLine("JSON not found");
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}
	json settings = json::parse(settingsLoc);
	if (settings.is_discarded()) {
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}

	//std::string imageLoc = "data/test.png";
	//std::string imageLoc = "data/grayscale.png";
	//std::string imageLoc = "data/lenna.png";
	std::string imageLoc = "data/alphaTest.png";
	//std::string imageLoc = "data/alphaTest-gradient.png";
	Image::ImageType imageType = Image::GetFileType(imageLoc.c_str());
	if (imageType == Image::ImageType::NA) {
		Log::WriteOneLine("Image not found");
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}

	std::string paletteLocStr = "data/wplace_premium.palette";
	//std::string paletteLocStr = "data/minecraft_map_sc.palette";
	//std::string paletteLocStr = "data/gameboy.palette";
	std::ifstream paletteLoc(paletteLocStr);
	if (!(paletteLoc)) {
		Log::WriteOneLine("Palette not found");
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}
#else
	if (argc < 4) {
		Log::WriteOneLine("Drag and drop an image file, a .palette file and a .json file");
		Log::WriteOneLine("Note: Only PNG, JPG, BMP or TGA image files are supported");

		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}

	json settings;
	std::string paletteLocStr;
	std::string imageLoc;
	for (int i = 0; i < argc; ++i) {
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
				return EXIT_FAILURE;
			}

			settings = json::parse(settingsLoc);
			if (settings.is_discarded()) {
				Log::Save();
				Log::HoldConsole();
				return EXIT_FAILURE;
			}
		} else if (extension == ".palette") {
			paletteLocStr = argv[i];
			std::ifstream paletteLoc(paletteLocStr);
			if (!(paletteLoc)) {
				Log::WriteOneLine("Palette not found");
				Log::Save();
				Log::HoldConsole();
				return EXIT_FAILURE;
			}
		} else {
			imageLoc = argv[i];
			Image::ImageType imageType = Image::GetFileType(imageLoc.c_str());
			if (imageType == Image::ImageType::NA) {
				Log::WriteOneLine("Image not found");
				Log::Save();
				Log::HoldConsole();
				return EXIT_FAILURE;
			}
		}
	}
	Log::EndLine();
#endif // _DEBUG

	// ========== GET SETTINGS ==========

	Log::WriteOneLine("===== GETTING SETINGS =====");
	std::unordered_map<std::string, json::value_t> required = {
		//{ "grayscale", json::value_t::boolean },
		//{ "dist_lightness", json::value_t::boolean },
		{ "ditherType", json::value_t::string },
		{ "distanceMode", json::value_t::string },
		{ "mathMode", json::value_t::string },
		{ "hideSemiTransparent", json::value_t::boolean },
		{ "hideThreshold", json::value_t::number_unsigned },
		{ "mono", json::value_t::boolean },
		{ "grayscale", json::value_t::boolean },
		{ "matrixType", json::value_t::string },
		{ "ditherAlpha", json::value_t::boolean},
		{ "ditherAlphaFactor", json::value_t::number_unsigned },
		{ "ditherAlphaType", json::value_t::string }
	};

	bool allFound = true;
	for (auto it = required.begin(); it != required.end(); ++it) {
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
		} else if (it->second == json::value_t::number_unsigned) {
			Log::WriteOneLine(it->first + ": " + Log::ToString(static_cast<unsigned int>(settings[it->first]), 0, '0'));
		}
	}

	if (!allFound) {
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}
	bool invalidType = false;
	if (settings["ditherType"] == "floyd" || settings["ditherType"] == "floyd-steinberg" ||
		settings["ditherType"] == "steinberg" || settings["ditherType"] == "fs") {
		settings["ditherType"] = "fs";
	} else if (settings["ditherType"] == "ordered") {
		settings["ditherType"] = "ordered";
	} else if (settings["ditherType"] == "none") {
		settings["ditherType"] = "none";
	} else {
		Log::WriteOneLine("Invalid ditherType: " + settings["ditherType"]);
		invalidType = true;
	}

	if (settings["ditherAlphaType"] == "floyd" || settings["ditherAlphaType"] == "floyd-steinberg" ||
		settings["ditherAlphaType"] == "steinberg" || settings["ditherAlphaType"] == "fs") {
		settings["ditherAlphaType"] = "fs";
	} else if (settings["ditherAlphaType"] == "ordered") {
		settings["ditherAlphaType"] = "ordered";
	} else if (settings["ditherAlphaType"] == "none") {
		settings["ditherAlphaType"] = "none";
	} else {
		Log::WriteOneLine("Invalid ditherAlphaType: " + settings["ditherAlphaType"]);
		invalidType = true;
	}

	if (!CheckColourMathMode(settings["distanceMode"])) {
		Log::WriteOneLine("Invalid distanceMode: " + settings["distanceMode"]);
		invalidType = true;
	}

	if (!CheckColourMathMode(settings["mathMode"])) {
		Log::WriteOneLine("Invalid mathMode: " + settings["mathMode"]);
		invalidType = true;
	}

	// No big noticeable difference between a 16x16 blue noise map vs a 32x32 blue noise map
	if (settings["matrixType"] != "bayer" && settings["matrixType"] != "bluenoise16"/* && settings["matrixType"] != "bluenoise32"*/) {
		Log::WriteOneLine("Invalid matrixType: " + settings["matrixType"]);
		invalidType = true;
	}

	if (invalidType) {
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}

	Dither::SetSettings(
		settings["distanceMode"], 
		settings["mathMode"], 
		settings["mono"], 
		settings["matrixType"], 
		((bool)settings["hideSemiTransparent"] ? false : (bool)settings["ditherAlpha"]),
		static_cast<unsigned int>(settings["ditherAlphaFactor"]),
		settings["ditherAlphaType"]);

	// ========== GET IMAGE ==========

	Log::EndLine();
	Log::WriteOneLine("===== GETTING IMAGE =====");

	Image image;
	if (!image.Read(imageLoc.c_str())) {
		Log::Save();
		Log::HoldConsole();
		return EXIT_FAILURE;
	}

	if (settings["mono"] || !settings["grayscale"]) {
		image.ToRGB();
	} else if (settings["grayscale"] && image.GetChannels() >= 3) {
		Dither::SetColourMathMode(settings["distanceMode"]);
		//Dither::ImageToGrayscale(image);
		Dither::ImageToGrayscale(image);

		// saves grayscale version
		std::string folder = NoExtension(imageLoc);
		std::filesystem::create_directories(folder);

		folder += "\\grayscale-" + (std::string)settings["distanceMode"] + ".png";

		image.Write(folder.c_str());
	}

	//if (settings["mono"]) image.ToRGB();
	Log::WriteOneLine("Is Grayscale: " + Log::ToString(image.IsGrayscale()));

	if ((bool)settings["hideSemiTransparent"]) image.HideSemiTransparent(settings["hideThreshold"]);

	// ========== GET PALETTE ==========

	Log::EndLine();
	Log::WriteOneLine("===== GETTING PALETTE =====");
	Palette palette(paletteLocStr.c_str());

	// ========== DITHERING ==========

	Log::EndLine();
	Log::WriteOneLine("===== DITHERING =====");

	if (settings["ditherType"] == "ordered") {
		Dither::OrderedDither(image, palette);
	} else if (settings["ditherType"] == "fs") {
		Dither::FloydDither(image, palette);
	} else {
		Dither::NoDither(image, palette);
	}

	// ===== Generate Output Path =====

	const std::string folder = NoExtension(imageLoc);

	std::string outputLoc = folder + '\\';

	if (settings["mono"]) {
		outputLoc += "mono";
	} else if (settings["grayscale"]) {
		outputLoc += "grayscale";
	} else {
		outputLoc += "regular";
	}

	std::filesystem::create_directories(outputLoc);

	outputLoc += "\\" + (std::string)settings["ditherType"];

	if (settings["ditherType"] == "ordered") outputLoc += "-" + (std::string)settings["matrixType"];

	outputLoc += "-" + (std::string)settings["distanceMode"];

	if (settings["ditherType"] != "none") outputLoc += "-" + (std::string)settings["mathMode"];

	outputLoc += ".png";

	image.Write(outputLoc.c_str());

#endif // DEV_MODE

	Log::Save();
	//Log::HoldConsole();
	Log::Sound(1);
	return EXIT_SUCCESS;
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

bool CheckColourMathMode(const std::string& mode) {
	if (mode == "srgb") {
		return true;
	} else if (mode == "oklab") {
		return true;
	} else if (mode == "oklab_l") {
		return true;
	} else if (mode == "lrgb") {
		return true;
	}
	return false;
}