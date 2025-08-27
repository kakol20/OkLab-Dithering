#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "image/Colour.h"
#include "image/Dither.h"
#include "image/Image.h"
#include "image/Palette.h"
#include "wrapper/Log.h"
#include "wrapper/Maths.hpp"

// https://json.nlohmann.me/home/exceptions/#switch-off-exceptions
#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception) { Log::WriteOneLine((exception).what()); }\

#include "../ext/json/json.hpp"
using json = nlohmann::json;

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
	if (settings.is_discarded()) {
		Log::Save();
		Log::HoldConsole();
		return -1;
	}

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
				return -1;
			}
			
			settings = json::parse(settingsLoc);
			if (settings.is_discarded()) {
				Log::Save();
				Log::HoldConsole();
				return -1;
			}
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
		//{ "grayscale", json::value_t::boolean },
		//{ "dist_lightness", json::value_t::boolean },
		{ "ditherType", json::value_t::string },
		{ "distanceMode", json::value_t::string },
		{ "mathMode", json::value_t::string },
		{ "hideSemiTransparent", json::value_t::boolean },
		{ "hideThreshold", json::value_t::number_unsigned },
		{ "mono", json::value_t::boolean },
		{ "grayscale", json::value_t::boolean }
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
			Log::WriteOneLine(it->first + ": " + Log::ToString((unsigned int)settings[it->first]));
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
	if (!(settings["mono"]) && settings["grayscale"] && image.GetChannels() >= 3) {
		// Convert image to grayscale
		if (settings["distanceMode"] == "srgb") {
			Colour::SetMathMode(Colour::MathMode::sRGB);
		} else {
			Colour::SetMathMode(Colour::MathMode::OkLab);
		}

		const int channels = image.GetChannels() == 3 ? 1 : 2;
		Image newImage(image.GetWidth(), image.GetHeight(), channels);

		for (int x = 0; x < image.GetWidth(); ++x) {
			for (int y = 0; y < image.GetHeight(); ++y) {
				const size_t newIndex = newImage.GetIndex(x, y);
				Colour col = Dither::GetColourFromImage(image, x, y);
				const double l_d = col.MonoGetLightness();
				
				if (Colour::GetMathMode() == Colour::MathMode::sRGB) {
					col.SetsRGB_D(l_d, l_d, l_d);
				} else {
					col.SetOkLab(l_d, 0., 0.);
				}

				newImage.SetData(newIndex, col.GetsRGB_UInt().r);

				if (channels == 2) {
					const size_t oldIndex = image.GetIndex(x, y) + 3;
					newImage.SetData(newIndex + 1, image.GetData(oldIndex));
				}
			}
		}

		image = newImage;

		// saves grayscale version
		std::string folder = NoExtension(imageLoc);
		std::filesystem::create_directories(folder);

		folder += "\\grayscale-" + (std::string)settings["distanceMode"] + ".png";

		image.Write(folder.c_str());
	}

	if (settings["mono"]) image.ToRGB();
	Log::WriteOneLine("Is Grayscale: " + Log::ToString(image.IsGrayscale()));

	if ((bool)settings["hideSemiTransparent"]) image.HideSemiTransparent(settings["hideThreshold"]);

	// ========== GET PALETTE ==========

	Log::EndLine();
	Log::WriteOneLine("===== GETTING PALETTE =====");
	Palette palette(paletteLocStr.c_str());

	// ========== DITHERING ==========

	Dither::SetSettings(settings["distanceMode"], settings["mathMode"], settings["mono"]);

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
	std::filesystem::create_directories(folder);

	std::string outputLoc = folder + '\\';

	if (settings["mono"]) outputLoc += "mono-";

	if (settings["ditherType"] == "none") {
		outputLoc +=
			(std::string)settings["ditherType"] + "-" +
			(std::string)settings["distanceMode"];
	} else {
		outputLoc +=
			(std::string)settings["ditherType"] + "-" +
			(std::string)settings["distanceMode"] + "-" +
			(std::string)settings["mathMode"];
	}
	if (image.IsGrayscale()) {
		outputLoc += "-grayscale.png";
	} else {
		outputLoc += ".png";
	}

	image.Write(outputLoc.c_str());

	Log::Save();
	//Log::HoldConsole();
	Log::Sound(1);
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