#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "../ext/json/json.hpp"
using json = nlohmann::json;

#include "image/Colour.h"
#include "image/Image.h"
#include "wrapper/Log.h"
#include "wrapper/Maths.hpp"

//const double Maths::Pi = 3.1415926535;
//const double Maths::Tau = 6.283185307;
//const double Maths::RadToDeg = 180. / Maths::Pi;
//const double Maths::DegToRad = Maths::Pi / 180.;

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
	Image::ImageType imageType = Image::GetFileType("data/suzanne.png");
	if (imageType == Image::ImageType::NA) {
		Log::WriteOneLine("Image not found");
		Log::Save();
		Log::HoldConsole();
		return -1;
	}

	std::ifstream paletteLoc("data/minecraft_map_sc.palette");
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
	std::array<std::string, 4> required = {
		"dither",
		"grayscale",
		"dist_lightness",
		"ordered_dither"
	};
	bool allFound = true;
	for (const auto& key : required) {
		if (!settings.contains(key)) {
			Log::WriteOneLine("JSON setting not found: " + key);
			allFound = false;
		} else {
			Log::WriteOneLine(key + ": " + Log::ToString((bool)settings[key]));
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

	// FOR TESTING
#ifdef _DEBUG
	Colour sRGB_t = Colour::FromsRGB(133, 172, 255);
	Colour OkLab_t = Colour::FromOkLab(0.75, -0.01, -0.13);

	sRGB_t.SetsRGB(128, 128, 255);

	Colour beforeFallback = Colour::FromOkLab(-0.5, 0.5, -0.5);

	Colour::SetMathMode(Colour::MathMode::OkLab_Lightness);

	Colour afterFallBack = beforeFallback;
	afterFallBack.Clamp();
	afterFallBack.UpdatesRGB();
	Colour::sRGB_UInt fallbackInt = afterFallBack.GetsRGB_UInt();

	double threshold = (255. + 0.5) / 256. - 0.5;
	Colour threshold_c = Colour::FromOkLab(threshold, threshold, threshold);
	threshold_c *= 1. / 16.;

	Colour dithered = afterFallBack + threshold_c;
	dithered.Clamp();
	dithered.UpdatesRGB();
	Colour::sRGB_UInt ditheredInt = dithered.GetsRGB_UInt();

	// Test save
	image.Clear();
	image.Write("data/output.png");
#endif // _NDEBUG

	Log::Save();
	Log::HoldConsole();
	return 0;
}