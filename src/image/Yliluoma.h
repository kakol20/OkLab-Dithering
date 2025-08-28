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
	
	/// <summary>
	/// Plan a 2-color mix for one pixel
	/// </summary>
	struct Plan2 {
		int i0 = -1, i1 = -1;
		double q = 0.;
	};

	struct MonoLimits {
		double min, max;
	};

	static std::string m_distanceMode, m_mathMode;
	static bool m_mono;
	static MonoLimits m_monoLimits;

	static Colour TosRGB(const LinearRGB& col);
	static LinearRGB ToLinearRGB(const Colour& col);

	static double Lum(const Colour& col);

	static double Dist2LAB(const Colour& a, const Colour& b);

	static double Dist2LRGB(const LinearRGB& a, const LinearRGB& b);

	static std::vector<int> KNearestLAB(const std::vector<Colour>& palL, const Colour& targetL, const int K);
	static std::vector<int> KNearestLRGB(const std::vector<LinearRGB>& palL, const LinearRGB& targetL, const int K);

	/// <summary>
	/// Plan search (Algorithm 1). Steps: test pairs among K nearest; scan 256 ratios.
	/// </summary>
	/// <param name="targetL"></param>
	/// <param name="palL"></param>
	/// <param name="K">Candidate set size</param>
	/// <param name="lamda">Psychovisual weight</param>
	/// <returns></returns>
	static Plan2 GetPlanLAB(const Colour& targetL, const std::vector<Colour>& palL, const int K = 16, const double lambda = 0.08);

	/// <summary>
	/// Plan search (Algorithm 1). Steps: test pairs among K nearest; scan 256 ratios.
	/// </summary>
	/// <param name="targetL"></param>
	/// <param name="palL"></param>
	/// <param name="K">Candidate set size</param>
	/// <param name="lamda">Psychovisual weight</param>
	/// <returns></returns>
	static Plan2 GetPlanLRGB(const LinearRGB& targetL, const std::vector<LinearRGB>& palL, const int K = 16, const double lambda = 0.08);
};

