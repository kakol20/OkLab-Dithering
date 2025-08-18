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
	/// <returns>Success</returns>
	static bool OrderedDither(Image& image, const Palette& palette);

private:
	static std::array<uint8_t, 256> m_bayer16;

	static inline size_t BayerIndex(const int x, const int y) { return size_t(x + y * 16); };

	static Colour ClosestColour(const Colour& col, const Palette& palette);
};

