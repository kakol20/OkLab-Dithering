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
	struct Plan2 {
		int i0 = -1, i1 = 1;
		double q = 0.;
	};

	static std::string m_distanceMode, m_mathMode;
	static bool m_mono;

	Colour TosRGB(const LinearRGB& col);
	LinearRGB ToLinearRGB(const Colour& col);

	static double Lum(const Colour& col);

	//static double Dist2LAB(const Colour& a, const Colour& b);
	static double Dist2LRGB(const LinearRGB& a, const LinearRGB& b);

	std::vector<int> KNearestLAB(const std::vector<Colour>& palL, const Colour& targetL, const int K);
	std::vector<int> KNearestLRGB(const std::vector<LinearRGB>& palL, const LinearRGB& targetL, const int K);
};

