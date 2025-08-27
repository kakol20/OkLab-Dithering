#include "Yliluoma.h"

#include <cmath>

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
