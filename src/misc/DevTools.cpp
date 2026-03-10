#ifdef DEV_MODE

#include "DevTools.h"

#include "../image/Colour.h"
#include "../image/Image.h"
#include "../image/Palette.h"
#include "../wrapper/Log.h"
#include "../wrapper/Threshold.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

void DevTools::Run() {
	//GenerateBlueNoise();
	GenerateBlueNoisePalette();
	Log::EndLine();
	Log::EndLine();
	Log::Clear();
	PaletteValues();
}

void DevTools::GenerateGSTiles() {
	//// https://github.com/Calinou/free-blue-noise-textures

	Image img(301, 301, 3);
	for (int x = 0; x < 301; ++x) {
		for (int y = 0; y < 301; ++y) {
			int tileX = x / 19;
			int tileY = y / 19;

			uint8_t gray = 0;

			if (x % 19 >= 16 || y % 19 >= 16) {
				gray = 0;
			} else {
				gray = static_cast<uint8_t>(tileY * 16 + tileX);
			}

			size_t index = img.GetIndex(x, y);

			img.SetData(index + 0, gray);
			img.SetData(index + 1, gray);
			img.SetData(index + 2, gray);
		}
	}
	img.Write("data/gs-tiles.png");
}

void DevTools::PaletteValues() {
	//Log::Write("Test\n");

	Palette palette = "data/custom.palette";
	//
	//for (size_t i = 0; i < palette.size(); ++i) {
	//	Log::Write(palette.GetColour(i).sRGBUintDebug());
	//	Log::EndLine();
	//}

	Log::Save("dev/colors.txt");
}

void DevTools::GenerateBlueNoisePalette() {
	std::vector<Colour> palette;
	palette.reserve(64);
	Colour::SetMathMode(Colour::MathMode::OkLab);

	// Set intial mandatory colours
	palette.emplace_back(0., 0., 0.);
	palette.emplace_back(0.2, 0., 0.);
	palette.emplace_back(0.4, 0., 0.);
	palette.emplace_back(0.6, 0., 0.);
	palette.emplace_back(0.8, 0., 0.);
	palette.emplace_back((uint8_t)255, 255, 255);
	palette.emplace_back((uint8_t)255, 0, 0);
	palette.emplace_back((uint8_t)0, 255, 0);
	palette.emplace_back((uint8_t)255, 255, 0);
	palette.emplace_back((uint8_t)0, 0, 255);
	palette.emplace_back((uint8_t)255, 0, 255);
	palette.emplace_back((uint8_t)0, 255, 255);

	// ========== SAVE COLOURS ==========
	Colour::SetMathMode(Colour::MathMode::OkLCh);
	std::sort(palette.begin(), palette.end());
	for (size_t i = 0; i < palette.size(); ++i) {
		Log::Write(palette[i].GetHex());
		
		if (i < palette.size() - 1) Log::EndLine();
	}
	Log::Save("data/custom.palette");
}

void DevTools::GenerateBlueNoise() {
	const std::string type = "bluenoise32";
	const int size = 32;

	Threshold blueNoise;
	blueNoise.GenerateThreshold(type);

	Image img(size, size, 1);
	for (int x = 0; x < size; ++x) {
		for (int y = 0; y < size; ++y) {
			double val = blueNoise.GetThreshold(x, y) + 0.5;
			val = std::floor(val * 255.);
			val = val > 255. ? 255. : val < 0. ? 0. : val;

			const size_t index = img.GetIndex(x, y);
			img.SetData(index, static_cast<uint8_t>(val));
		}
	}

	std::string outputLoc = "dev/" + type + ".png";
	img.Write(outputLoc.c_str());
}
#endif // DEV_MODE
