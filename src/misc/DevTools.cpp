#ifdef DEV_MODE

#include "DevTools.h"

#include "../image/Colour.h"
#include "../image/Image.h"
#include "../image/Palette.h"
#include "../misc/Random.h"
#include "../wrapper/Log.h"
#include "../wrapper/Threshold.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <limits>

void DevTools::Run() {
	//GenerateBlueNoise();
	GenerateBlueNoisePalette();
	Log::EndLine();
	Log::EndLine();
	Log::Clear();
	PaletteValues();

	//Misc();
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

	Palette palette = "data/custom64.palette";
	
	// average distance to nearest
	Colour::SetMathMode(Colour::MathMode::OkLab);
	double total = 0.;
	for (size_t i = 0; i < palette.size(); ++i) {
		double nearestDist = -1;

		for (size_t j = 0; j < palette.size(); ++j) {
			if (i == j) continue;
			if (nearestDist < 0) {
				nearestDist = palette.GetColour(i).MagSq(palette.GetColour(j));
				continue;
			}

			double currentDist = palette.GetColour(i).MagSq(palette.GetColour(j));
			if (currentDist < nearestDist) nearestDist = currentDist;
		}

		total += nearestDist;
	}
	total /= static_cast<double>(palette.size());
	Log::WriteOneLine("Average Distance to Nearest: " + Log::ToString(total, 4));

	Log::Save("dev/colors.txt");
}

void DevTools::GenerateBlueNoisePalette() {
	std::vector<Colour> palette;
	const size_t size = 64;
	palette.reserve(size);

	//palette.emplace_back(1. / 7., 0., 0.);
	//palette.emplace_back(2. / 7., 0., 0.);
	//palette.emplace_back(3. / 7., 0., 0.);
	//palette.emplace_back(4. / 7., 0., 0.);
	//palette.emplace_back(5. / 7., 0., 0.);
	//palette.emplace_back(6. / 7., 0., 0.);

	palette.emplace_back(0.2, 0., 0.);
	palette.emplace_back(0.4, 0., 0.);
	palette.emplace_back(0.6, 0., 0.);
	palette.emplace_back(0.8, 0., 0.);

	// Set intial mandatory colours
	palette.emplace_back(0., 0., 0.);
	palette.emplace_back((uint8_t)255, 255, 255);
	palette.emplace_back((uint8_t)255, 0, 0);
	palette.emplace_back((uint8_t)255, 255, 0);
	palette.emplace_back((uint8_t)0, 255, 0);
	palette.emplace_back((uint8_t)0, 255, 255);
	palette.emplace_back((uint8_t)0, 0, 255);
	palette.emplace_back((uint8_t)255, 0, 255);

	// ========== GENERATE ==========

	const size_t m = 5;
	Random::Seed = 0;

	Colour::SetMathMode(Colour::MathMode::OkLab);
	const size_t startSize = palette.size();
	for (size_t i = startSize; i < size; ++i) {
		Colour furthestCol;
		double furthestDist = 0.;
		
		const size_t candidateCount = palette.size() * m + 1;

		for (size_t j = 0; j < candidateCount; ++j) {
			const uint8_t currColR = static_cast<uint8_t>(Random::RandUInt(0, 255));
			const uint8_t currColG = static_cast<uint8_t>(Random::RandUInt(0, 255));
			const uint8_t currColB = static_cast<uint8_t>(Random::RandUInt(0, 255));

			if (currColR == 48 && currColG == 169 && currColB == 32) {
				bool temp = true;
			}

			Colour currCol(currColR, currColG, currColB);
			double closestDist = 0;

			for (size_t k = 0; k < palette.size(); ++k) {
				double currentDist = currCol.MagSq(palette[k]);

				if (k == 0) closestDist = currentDist;
				if (currentDist < closestDist) closestDist = currentDist;
			}

			if (closestDist > furthestDist) {
				furthestCol = currCol;
				furthestDist = closestDist;
			}
		}

		palette.push_back(furthestCol);
	}

	// ========== SAVE COLOURS ==========
	// As palette file
	Colour::SetMathMode(Colour::MathMode::OkLCh);
	std::sort(palette.begin(), palette.end());
	for (size_t i = 0; i < palette.size(); ++i) {
		Log::Write(palette[i].GetHex());
		
		if (i < palette.size() - 1) Log::EndLine();
	}
	Log::Save("data/custom" + Log::ToString(size) + ".palette");

	// As Image
	const double size_d = std::ceil(std::sqrt(static_cast<double>(palette.size())));
	const int size_i = static_cast<int>(size_d);

	const int width = static_cast<int>(std::ceil(std::sqrt(palette.size())));
	const int height = static_cast<int>(std::ceil(static_cast<double>(palette.size()) / width));

	Image palImg(width, height, 4);
	palImg.Clear();
	for (size_t i = 0; i < palette.size(); ++i) {
		size_t imgI = i * 4;
		Colour::sRGB_UInt srgbUint = palette[i].GetsRGB_UInt();

		palImg.SetData(imgI + 0, srgbUint.r);
		palImg.SetData(imgI + 1, srgbUint.g);
		palImg.SetData(imgI + 2, srgbUint.b);
		palImg.SetData(imgI + 3, 255);
	}

	palImg.Write(("dev/custom" + Log::ToString(size) + "-pal.png").c_str());
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
void DevTools::Misc() {
	Colour::SetMathMode(Colour::MathMode::OkLab);
	Colour col1((uint8_t)0, 0, 0);
	Colour col2((uint8_t)0, 0, 255);

	double dist = std::sqrt(col1.MagSq(col2));
	double count = std::ceil(dist / 0.2) + 1;
	Log::WriteOneLine(Log::ToString(dist));
	Log::WriteOneLine(Log::ToString(count));

	Log::HoldConsole();
}
#endif // DEV_MODE
