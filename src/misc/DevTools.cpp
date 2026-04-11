#ifdef DEV_MODE

#include "DevTools.h"

#include "../../ext/json/json.hpp"
#include "../../res/resource.h"
#include "../image/Colour.h"
#include "../image/Image.h"
#include "../image/Palette.h"
#include "../misc/Random.h"
#include "../wrapper/Log.h"
#include "../wrapper/Threshold.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <cstring>
#include <vector>
#include <windows.h>

#include "../misc/BN_Helper.h"

using json = nlohmann::json;

void DevTools::Run() {
	//ThresholdToImage();
	//GenerateBlueNoisePalette();
	//Log::EndLine();
	//Log::EndLine();
	//Log::Clear();
	//PaletteValues();
	//DebugThreshold();

	//GenerateBlueNoise(16, "res/blueNoise16.bin");
	//GenerateBlueNoise(32, "res/blueNoise32.bin");
	//GenerateBlueNoise(64, "res/blueNoise64.bin");
	//GenerateBlueNoise(128, "res/blueNoise128.bin");
	ReadBlueNoiseBin(IDI_BN16);
	ReadBlueNoiseBin(IDI_BN32);
	ReadBlueNoiseBin(IDI_BN64);
	ReadBlueNoiseBin(IDI_BN128);

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

void DevTools::ThresholdToImage() {
	const int size = 128;
	//const std::string type = "bluenoise" + Log::ToString(size);
	const std::string type = "ign";


	Threshold blueNoise;
	blueNoise.GenerateThreshold(type);

	double min = 255.;
	double max = 0.;

	Image img(size, size, 1);

	// get min and max
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double val = blueNoise.GetThreshold(x, y) + 0.5;
			
			if (val < min) min = val;
			if (val > max) max = val;
		}
	}

	// save to image and normalise
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double val = blueNoise.GetThreshold(x, y) + 0.5;
			val = (val - min) / (max - min);
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

void DevTools::DebugThreshold() {
	std::ifstream settingsLoc("data/settings.json");
	if (!(settingsLoc)) {
		Log::WriteOneLine("JSON not found");
		Log::Save();
		Log::HoldConsole();
		return;
	}
	json settings = json::parse(settingsLoc);
	if (settings.is_discarded()) {
		Log::Save();
		Log::HoldConsole();
		return;
	}

	std::vector<int> sizes;
	settings["shape"]["size"].get_to(sizes);

	std::vector<std::vector<int>> points;
	settings["shape"]["points"].get_to(points);

	Threshold::SetShape(sizes[0], sizes[1], points);

	Threshold threshold;
	threshold.GenerateThreshold("bayershape16");

	Image img(sizes[0] * 16, sizes[1] * 16, 3);

	for (int x = 0; x < img.GetWidth(); ++x) {
		for (int y = 0; y < img.GetHeight(); ++y) {
			if (x == 1 && y == 0) {
				bool temp = true;
			}

			const size_t index = img.GetIndex(x, y);
			const double value = threshold.GetThreshold(x, y) + 0.5;
			const Colour col(value, 0., 0.);
			const uint8_t colVal = col.GetsRGB_UInt().r;

			img.SetData(index + 0, colVal);
			img.SetData(index + 1, colVal);
			img.SetData(index + 2, colVal);
		}
	}

	img.Write("dev/bayershape16.png");
}

void DevTools::GenerateBlueNoise(const uint32_t size, const char* filename) {
	Random::Seed = 20260410;

	const std::filesystem::path p = filename;
	const std::filesystem::path dir = p.parent_path();
	if (!p.parent_path().empty() && !std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	std::ofstream out(filename, std::ios::binary);
	if (!out) return;

	std::vector<uint32_t> data = BN_Helper::Generate((int)size, Random::Seed);

	uint32_t dataSize = static_cast<uint32_t>(data.size());
	out.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
	out.write(reinterpret_cast<const char*>(data.data()), dataSize * sizeof(uint32_t));
}

void DevTools::ReadBlueNoiseBin(const int res) {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(res), RT_RCDATA);
	if (!hRes) return;

	HGLOBAL hData = LoadResource(NULL, hRes);
	void* pData = LockResource(hData);
	DWORD dataSize = SizeofResource(NULL, hRes);
	if (!pData || dataSize < sizeof(uint32_t)) return;

	const char* bytes = reinterpret_cast<const char*>(pData);

	// Read the size first
	uint32_t size = 0;
	std::memcpy(&size, bytes, sizeof(uint32_t));

	// Check that the resource size matches expected
	if (dataSize < sizeof(uint32_t) + size * sizeof(uint32_t)) return;

	std::vector<uint32_t> result(size);
	std::memcpy(result.data(), bytes + sizeof(uint32_t), size * sizeof(uint32_t));

	int imgSize = (int)std::sqrt(size);

	Log::WriteOneLine("size: " + Log::ToString((unsigned int)size, 3, ' '));
	Image img(imgSize, imgSize, 1);

	for (size_t i = 0; i < (size_t)size; ++i) {
		double value = static_cast<double>(result[i]) / (size - 1);
		value = std::floor(value * 256.);
		value = value >= 256. ? 255. : value;

		//const int x = i % imgSize;
		//const int y = i / imgSize;

		//Log::WriteOneLine(Log::ToString((unsigned int)result[i], 3, ' '));
		img.SetData(i, static_cast<uint8_t>(value));
	}

	std::string outLoc = "dev/res/blueNoise" + Log::ToString(imgSize) + ".png";
	img.Write(outLoc.c_str());
}
#endif // DEV_MODE