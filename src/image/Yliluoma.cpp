#include "Yliluoma.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "../wrapper/Log.h"
#include "../wrapper/Maths.hpp"
#include "Dither.h"

std::string Yliluoma::m_distanceMode = "oklab";
std::string Yliluoma::m_mathMode = "srgb";
bool Yliluoma::m_mono = false;
Yliluoma::MonoLimits Yliluoma::m_monoLimits = { 0., 1. };

void Yliluoma::Run(Image& image, const Palette& palette) {
	if (m_mathMode == "srgb") {
		std::vector<LinearRGB> palL;
		palL.reserve(palette.size());
		for (size_t i = 0; i < palette.size(); ++i) {
			const Colour palC = palette.GetIndex(i);
			if (image.IsGrayscale() && !palC.IsGrayscale()) continue;
			palL.emplace_back(ToLinearRGB(palC));
		}

		Log::StartTime();
		Log::WriteOneLine("Yliluoma Ordered Dither...");

		for (int x = 0; x < image.GetWidth(); ++x) {
			for (int y = 0; y < image.GetHeight(); ++y) {
				const Colour col = Dither::GetColourFromImage(image, x, y);
				const LinearRGB targetL = ToLinearRGB(col);

				Plan2 plan = GetPlanLRGB(targetL, palL);

				const double t = (double)Dither::Bayer_16x16[Dither::MatrixIndex(x % 16, y % 16)] / 256.;

				Colour outCol = TosRGB(palL[(t < plan.q) ? plan.i1 : plan.i0]);
				Dither::SetColourToImage(outCol, image, x, y);

				// -- Check Time --
				if (Log::CheckTimeSeconds(5.)) {
					double progress = double(image.GetIndex(x, y)) / double(image.GetSize());
					progress *= 100.;

					std::string outStr = Log::ToString(progress, 6);
					outStr = Log::LeadingCharacter(outStr, 9);

					Log::WriteOneLine("\t" + outStr + "%");

					Log::StartTime();
				}
			}
		}
	} else {
		std::vector<Colour> palL;
		palL.reserve(palette.size());

		for (size_t i = 0; i < palette.size(); ++i) {
			const Colour palC = palette.GetIndex(i);
			if (image.IsGrayscale() && !palC.IsGrayscale()) continue;
			palL.emplace_back(palC);
		}

		Log::StartTime();
		Log::WriteOneLine("Yliluoma Ordered Dither...");

		for (int x = 0; x < image.GetWidth(); ++x) {
			for (int y = 0; y < image.GetHeight(); ++y) {
				const Colour targetL = Dither::GetColourFromImage(image, x, y);
				Plan2 plan = GetPlanLAB(targetL, palL);

				const double t = (double)Dither::Bayer_16x16[Dither::MatrixIndex(x % 16, y % 16)] / 256.;

				Colour outCol = palL[(t < plan.q) ? plan.i1 : plan.i0];
				Dither::SetColourToImage(outCol, image, x, y);

				// -- Check Time --
				if (Log::CheckTimeSeconds(5.)) {
					double progress = double(image.GetIndex(x, y)) / double(image.GetSize());
					progress *= 100.;

					std::string outStr = Log::ToString(progress, 6);
					outStr = Log::LeadingCharacter(outStr, 9);

					Log::WriteOneLine("\t" + outStr + "%");

					Log::StartTime();
				}
			}
		}
	}
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
0.0030857681800844569
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

double Yliluoma::Dist2LAB(const Colour& a, const Colour& b) {
	if (m_mono) return a.MonoDistance(b, m_monoLimits.min, m_monoLimits.max);
	return a.MagSq(b);
}

double Yliluoma::Dist2LRGB(const LinearRGB& a, const LinearRGB& b) {
	if (m_mono) {
		Colour aCol, bCol;
		aCol.SetsRGB_D(a.r, a.g, a.b);
		bCol.SetsRGB_D(b.r, b.g, b.b);
		return aCol.MonoDistance(bCol, m_monoLimits.min, m_monoLimits.max);
	}
	constexpr double wr = 0.2126, wg = 0.7152, wb = 0.0722;

	const double dr = a.r - b.r, dg = a.g - b.g, db = a.b - b.b;
	return wr * dr * dr + wg * dg * dg + wb * db * db;
}

std::vector<int> Yliluoma::KNearestLAB(const std::vector<Colour>& palL, const Colour& targetL, const int K) {
	std::vector<int> idx(palL.size());
	std::iota(idx.begin(), idx.end(), 0);

	std::partial_sort(idx.begin(), idx.begin() + std::min(K, (int)idx.size()), idx.end(),
		[&](int a, int b) {
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

Yliluoma::Plan2 Yliluoma::GetPlanLAB(const Colour& targetL, const std::vector<Colour>& palL, const int K, const double lambda) {
	if (m_distanceMode == "srgb") {
		Colour::SetMathMode(Colour::MathMode::sRGB);
	} else {
		Colour::SetMathMode(Colour::MathMode::OkLab);
	}

	std::vector<int> cand = KNearestLAB(palL, targetL, K);

	double best = 1e30;
	Plan2 plan{};

	constexpr int STEPS = 16 * 16; // for bayer matrix
	for (size_t a = 0; a < cand.size(); ++a) {
		for (size_t b = a + 1; b < cand.size(); ++b) {
			int i0 = cand[a], i1 = cand[b];
			Colour p0 = palL[i0], p1 = palL[i1];

			if (m_distanceMode == "srgb") {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			} else {
				Colour::SetMathMode(Colour::MathMode::OkLab);
			}
			double pair_d = Dist2LAB(p0, p1);
			if (!m_mono) pair_d = std::sqrt(pair_d);
			pair_d += 1e-8;

			for (int s = 0; s <= STEPS; ++s) {
				const double q = double(s) / double(STEPS);

				Colour::SetMathMode(Colour::MathMode::OkLab);
				Colour mix = (p0 * (1. - q)) + (p1 * q);
				mix.UpdatesRGB();

				if (m_distanceMode == "srgb") {
					Colour::SetMathMode(Colour::MathMode::sRGB);
				} else {
					Colour::SetMathMode(Colour::MathMode::OkLab);
				}

				double penalty = Dist2LAB(mix, targetL);
				double uneven = 0.5 * std::abs(q - 0.5);
				penalty += lambda * pair_d * uneven;
				if (penalty < best) {
					best = penalty;
					plan = { i0, i1, q };
				}
			}
		}
	}
	return plan;
}

Yliluoma::Plan2 Yliluoma::GetPlanLRGB(const LinearRGB& targetL, const std::vector<LinearRGB>& palL, const int K, const double lambda) {
	std::vector<int> cand;
	if (m_distanceMode == "srgb") {
		Colour::SetMathMode(Colour::MathMode::sRGB);
		cand = KNearestLRGB(palL, targetL, K);
	} else {
		Colour::SetMathMode(Colour::MathMode::OkLab);

		const Colour targetC = TosRGB(targetL);
		std::vector<Colour> palC;
		palC.reserve(palL.size());
		for (size_t i = 0; i < palL.size(); ++i) {
			palC.emplace_back(TosRGB(palL[i]));
		}

		cand = KNearestLAB(palC, targetC, K);
	}

	double best = 1e30;

	Plan2 plan{};
	constexpr int STEPS = 16 * 16; // for bayer matrix

	for (size_t a = 0; a < cand.size(); ++a) {
		for (size_t b = a + 1; b < cand.size(); ++b) {
			int i0 = cand[a], i1 = cand[b];
			LinearRGB p0 = palL[i0], p1 = palL[i1];

			double pair_d = 0.;
			if (m_distanceMode == "srgb") {
				Colour::SetMathMode(Colour::MathMode::sRGB);
				pair_d = Dist2LRGB(p0, p1);
			} else {
				Colour::SetMathMode(Colour::MathMode::OkLab);
				const Colour p0C = TosRGB(p0);
				const Colour p1C = TosRGB(p1);
				pair_d = Dist2LAB(p0C, p1C);
			}
			if (!m_mono) pair_d = std::sqrt(pair_d);
			pair_d += 1e-8;
			//double pair_d = std::sqrt(Dist2LRGB(p0, p1)) + 1e-8;

			for (int s = 0; s <= STEPS; ++s) {
				double q = double(s) / double(STEPS);

				// No need to use m_mathMode since it is called before function is called
				LinearRGB mix{
					(1 - q) * p0.r + q * p1.r,
					(1 - q) * p0.g + q * p1.g,
					(1 - q) * p0.b + q * p1.b
				};

				double penalty = 0.;
				if (m_distanceMode == "srgb") {
					Colour::SetMathMode(Colour::MathMode::sRGB);
					penalty = Dist2LRGB(mix, targetL);
				} else {
					Colour::SetMathMode(Colour::MathMode::OkLab);
					const Colour mixLab = TosRGB(mix);
					const Colour targetLab = TosRGB(targetL);
					penalty = Dist2LAB(mixLab, targetLab);
				}

				// Psychovisual term: discourage very uneven mixes of very distant colors.
				// Weight grows as |q-0.5| increases (uneven) and as colors differ.

				double uneven = 0.5 + std::abs(q - 0.5);
				penalty += lambda * pair_d * uneven;
				if (penalty < best) {
					best = penalty;
					plan = { i0, i1, q };
				}
			}
		}
	}
	return plan;
}