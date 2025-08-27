#include "Yliluoma.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "../wrapper/Maths.hpp"

std::string Yliluoma::m_distanceMode = "oklab";
std::string Yliluoma::m_mathMode = "srgb";
bool Yliluoma::m_mono = false;

void Yliluoma::Run(Image& image, const Palette& palette) {
}

void Yliluoma::SetSettings(const std::string distanceMode, const std::string mathMode, const bool mono) {
	m_distanceMode = distanceMode;
	m_mathMode = mathMode;
	m_mono = mono;

}

/*
2.4125093745073549,
0.056317370387926696,
0.039870440086508217,
12.920750283132739
// 0.0030857681800844569
*/

Colour Yliluoma::TosRGB(const LinearRGB& col) {
	constexpr double Y = 2.4125093745073549;
	constexpr double C = 0.056317370387926696;
	constexpr double X = 0.0030857681800844569;
	constexpr double A = 12.920750283132739;

	const double r = col.r <= X ? (col.r * A)
		: (C + 1.) * Maths::NRoot(col.r, Y) - C;
	const double g = col.g <= X ? (col.g * A)
		: (C + 1.) * Maths::NRoot(col.g, Y) - C;
	const double b = col.b <= X ? (col.b * A)
		: (C + 1.) * Maths::NRoot(col.b, Y) - C;

	return Colour::FromsRGB_D(r, g, b, col.a);
}

Yliluoma::LinearRGB Yliluoma::ToLinearRGB(const Colour& col) {
	constexpr double Y = 2.4125093745073549;
	constexpr double C = 0.056317370387926696;
	constexpr double X = 0.039870440086508217;
	constexpr double A = 12.920750283132739;

	Colour::sRGB colsRGB = col.GetsRGB();

	const double r = colsRGB.r <= X ? (colsRGB.r / A)
		: std::pow((colsRGB.r + C) / (1. + C), Y);
	const double g = colsRGB.g <= X ? (colsRGB.g / A)
		: std::pow((colsRGB.g + C) / (1. + C), Y);
	const double b = colsRGB.b <= X ? (colsRGB.b / A)
		: std::pow((colsRGB.b + C) / (1. + C), Y);
	return { r, g, b, col.GetAlpha() };
}

double Yliluoma::Lum(const Colour& col) {
	return col.MonoGetLightness();
}

//double Yliluoma::Dist2LAB(const Colour& a, const Colour& b) {
//	return a.MagSq(b);
//}

double Yliluoma::Dist2LRGB(const LinearRGB& a, const LinearRGB& b) {
	constexpr double wr = 0.2126, wg = 0.7152, wb = 0.0722;

	const double dr = a.r - b.r, dg = a.g - b.g, db = a.b - b.b;
	return wr * dr * dr + wg * dg * dg + wb * db * db;
}

std::vector<int> Yliluoma::KNearestLAB(const std::vector<Colour>& palL, const Colour& targetL, const int K) {
	std::vector<int> idx(palL.size());
	std::iota(idx.begin(), idx.end(), 0);

	std::partial_sort(idx.begin(), idx.begin() + std::min(K, (int)idx.size()), idx.end(),
		[&](int a, int b) {
			//return dist2()
			return palL[a].MagSq(targetL) < palL[b].MagSq(targetL);
		});
	idx.resize(std::min(K, (int)idx.size()));
	return idx;
}

std::vector<int> Yliluoma::KNearestLRGB(const std::vector<LinearRGB>& palL, const LinearRGB& targetL, const int K) {
	std::vector<int> idx(palL.size());
	std::iota(idx.begin(), idx.end(), 0);

	std::partial_sort(idx.begin(), idx.begin() + std::min(K, (int)idx.size()), idx.end(),
		[&](int a, int b) {
			//return dist2()
			return Dist2LRGB(palL[a], targetL) < Dist2LRGB(palL[b], targetL);
		});
	idx.resize(std::min(K, (int)idx.size()));
	return idx;
}
