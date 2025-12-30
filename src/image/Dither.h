#pragma once
#include "Colour.h"
#include "Image.h"
#include "Palette.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class Dither {
public:
	Dither() {};
	~Dither() {};

	/// <summary>
	/// Bayer Ordered Dithering
	/// </summary>
	/// <param name="image"></param>
	/// <param name="palette"></param>
	static void OrderedDither(Image& image, const Palette& palette);

	/// <summary>
	/// Floyd-Steinberg Dithering
	/// </summary>
	/// <param name="image"></param>
	/// <param name="palette"></param>
	/// <param name="distanceType"></param>
	static void FloydDither(Image& image, const Palette& palette);

	static void NoDither(Image& image, const Palette& palette);

	static void SetSettings(const std::string distanceType,
		const std::string mathMode,
		const bool mono,
		const std::string matrixType, const bool ditherAlpha, const unsigned int ditherAlphaFactor, const std::string ditherAlphaType);

	static Colour GetColourFromImage(const Image& image, const int x, const int y);
	static void SetColourToImage(const Colour& colour, Image& image, const int x, const int y);

	static void ImageToGrayscale(Image& image);

	static void SetColourMathMode(const std::string& mode);

private:
	static std::array<uint8_t, 1024> m_blueNoise32;
	static std::array<uint8_t, 256> m_bayer16;
	static std::array<uint8_t, 256> m_blueNoise16;

	static inline size_t MatrixIndex(const int x, const int y, const int size) { return size_t(x + y * size); };

	static Colour ClosestColour(const Colour& col, const Palette& palette, const bool grayscale = false);

	static std::string m_distanceMode, m_mathMode, m_matrixType, m_ditherAlphaType;
	static bool m_mono, m_ditherAlpha;
	static unsigned int m_ditherAlphaFactor;

	static double GetThreshold(const int x, const int y);

	//static void DitherAlphaChannel(Image& image, const int x, const int y);
	static void DitherAlpha(Colour& col, std::vector<Colour>& colours, const int x, const int y, const int imgWidth, const int imgHeight);
};
