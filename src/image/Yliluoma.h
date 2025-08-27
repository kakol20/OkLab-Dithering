#pragma once

#include "Colour.h"
#include "Image.h"
#include "Palette.h"

/// <summary>
/// <para>Joel Yliluoma's arbitrary-palette positional dithering algorithm</para>
/// <para>https://bisqwit.iki.fi/story/howto/dither/jy/</para>
/// </summary>
class Yliluoma {
public:
	Yliluoma() {};
	~Yliluoma() {};

	static void Run(Image& image, const Palette& palette);

	static void SetSettings(const std::string distanceMode, const std::string mathMode, const bool mono);

private:
	struct LinearRGB {
		double r, g, b, a;
	};

	Colour TosRGB(const LinearRGB& col);
	LinearRGB ToLinearRGB(const Colour& col);

	static std::string m_distanceMode, m_mathMode;
	static bool m_mono;
};

