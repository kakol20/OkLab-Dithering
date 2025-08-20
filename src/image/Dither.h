#pragma once
#include <array>
#include "Image.h"
#include "Palette.h"

class Dither {
public:
	Dither() {};
	~Dither() {};

	/// <summary>
	/// Bayer Ordered Dithering
	/// </summary>
	/// <param name="image"></param>
	/// <param name="palette"></param>
	static void OrderedDither(Image& image, const Palette& palette, const std::string distanceType = "oklab", const std::string mathMode = "srgb");

	/// <summary>
	/// Floyd-Steinberg Dithering
	/// </summary>
	/// <param name="image"></param>
	/// <param name="palette"></param>
	/// <param name="distanceType"></param>
	static void FloydDither(Image& image, const Palette& palette, const std::string distanceType = "oklab", const std::string mathMode = "srgb");

	static void NoDither(Image& image, const Palette& palette, const std::string distanceType = "oklab");

private:
	static std::array<uint8_t, 256> m_bayer16;

	static inline size_t MatrixIndex(const int x, const int y) { return size_t(x + y * 16); };

	static Colour ClosestColour(const Colour& col, const Palette& palette, const bool grayscale = false);

	static Colour GetColourFromImage(const Image& image, const int x, const int y);
	static void SetColourToImage(const Colour& colour, Image& image, const int x, const int y);
};

