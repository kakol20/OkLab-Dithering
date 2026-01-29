#include "../wrapper/Log.h"
#include "../wrapper/Threshold.h"
#include "Colour.h"
#include "Dither.h"
#include "Image.h"
#include "Palette.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

bool Dither::m_mono = false;
bool Dither::m_ditherAlpha = false;
std::string Dither::m_distanceMode = "oklab";
std::string Dither::m_mathMode = "srgb";
std::string Dither::m_matrixType = "bayer";
std::string Dither::m_ditherAlphaType = "ordered";
unsigned int Dither::m_ditherAlphaFactor = 1;

void Dither::OrderedDither(Image& image, const Palette& palette) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	// Create a copy of of image in Colour form
	const size_t coloursSize = (size_t)image.GetHeight() * image.GetWidth();
	std::vector<Colour> colours;
	colours.reserve(coloursSize);

	Log::StartTime();
	Log::WriteOneLine("ORDERED DITHERING...");

	Log::WriteOneLine("  Copying Pixels");

	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t index = image.GetIndex(x, y);
			//Colour pixel = GetColourFromImage(image, x, y);
			colours.emplace_back(GetColourFromImage(image, x, y));

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}

	struct DitherInfo {
		double alpha = 0.;
		Colour p0;
		Colour p1;
	};

	std::map<Colour, DitherInfo> ditherMem;

	SetColourMathMode(m_distanceMode);

	Log::WriteOneLine("  Dithering");
	for (int x = 0; x < imgWidth; ++x) {
		for (int y = 0; y < imgHeight; ++y) {
			const size_t indexCol = size_t(x + y * imgWidth);
			const size_t index = image.GetIndex(x, y);

			Colour pixel = colours[indexCol];

			//SetColourMathMode(m_mathMode);

			// ===== CHECK MEMOISATION =====

			DitherInfo info;
			
			if (ditherMem.find(pixel) != ditherMem.end()) {
				// Found
				info = ditherMem[pixel];
			} else if (palette.size() <= 1) {
				info.p0 = palette.GetColour(0);
				info.p1 = palette.GetColour(1);

				// Calculate Alpha value
				Colour d = info.p1 - info.p0;
				double denom = d.LengthSq();

				info.alpha = d.Dot(pixel - info.p0);
				info.alpha /= denom;
				info.alpha = std::clamp(info.alpha, 0., 1.);

				ditherMem[pixel] = info;
			}
			else {
				// Find p0 and p1
				
				size_t i0 = 0, i1 = 1;
				double d0 = pixel.MagSq(palette.GetColour(0));
				double d1 = pixel.MagSq(palette.GetColour(1));

				if (d1 < d0) {
					std::swap(d0, d1);
					std::swap(i0, i1);
				}

				for (size_t i = 2; i < palette.size(); ++i) {
					double d = pixel.MagSq(palette.GetColour(i));

					if (d < d0) {
						d1 = d0; i1 = i0;
						d0 = d;  i0 = i;
					} else if (d < d1) {
						d1 = d;  i1 = i;
					}
				}

				info.p0 = palette.GetColour(i0);
				info.p1 = palette.GetColour(i1);

				// Calculate Alpha value
				Colour d = info.p1 - info.p0;
				double denom = d.LengthSq();

				info.alpha = d.Dot(pixel - info.p0);
				info.alpha /= denom;
				info.alpha = std::clamp(info.alpha, 0., 1.);

				ditherMem[pixel] = info;
			}

			// ===== APPLY DITHER =====

			// This is to cancel out the (-0.5) inside GetThreshold() function
			const double threshold = Threshold::GetThreshold(x, y) + 0.5;

			//SetColourMathMode(m_distanceMode);
			//Colour nearest = ClosestColour(dithered, palette, image.IsGrayscale());

			Colour nearest = info.alpha > threshold ? info.p1 : info.p0;
			nearest.SetAlpha(pixel.GetAlpha());
			//Colour nearest = pixel;

			if (image.HasAlphaChannel() && m_ditherAlpha) DitherAlpha(nearest, colours, x, y, imgWidth, imgHeight);

			SetColourToImage(nearest, image, x, y);

			//if (m_ditherAlpha) DitherAlphaChannel(image, x, y);

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}

	Log::WriteOneLine("    Dither Mem Size: " + Log::ToString(ditherMem.size()));
}

void Dither::FloydDither(Image& image, const Palette& palette) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	// Create a copy of of image in Colour form
	const size_t coloursSize = (size_t)image.GetHeight() * image.GetWidth();
	std::vector<Colour> colours;
	colours.reserve(coloursSize);

	Log::StartTime();
	Log::WriteOneLine("FLOYD STEINBERG DITHERING...");

	Log::WriteOneLine("  Copying Pixels");
	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t index = image.GetIndex(x, y);
			//Colour pixel = GetColourFromImage(image, x, y);
			colours.emplace_back(GetColourFromImage(image, x, y));

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}

	// Dither
	Log::WriteOneLine("  Dithering");
	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t indexCol = size_t(x + y * imgWidth);
			const size_t index = image.GetIndex(x, y);

			Colour oldPixel = colours[indexCol];

			SetColourMathMode(m_distanceMode);
			Colour newPixel = ClosestColour(oldPixel, palette, image.IsGrayscale());
			if (image.HasAlphaChannel() && m_ditherAlpha) DitherAlpha(newPixel, colours, x, y, imgWidth, imgHeight);

			SetColourToImage(newPixel, image, x, y);

			SetColourMathMode(m_mathMode);
			Colour quantError = oldPixel - newPixel;

			size_t neighbourIndex = 0;

			if (x + 1 < imgWidth) {
				neighbourIndex = size_t((x + 1) + y * imgWidth);
				colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (7. / 16.));
				colours[neighbourIndex].Clamp();
				colours[neighbourIndex].Update();
			}

			if (y + 1 < imgHeight) {
				if (x - 1 >= 0) {
					neighbourIndex = size_t((x - 1) + (y + 1) * imgWidth);
					colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (3. / 16.));
					colours[neighbourIndex].Clamp();
					colours[neighbourIndex].Update();
				}
				if (x + 1 < imgWidth) {
					neighbourIndex = size_t((x + 1) + (y + 1) * imgWidth);
					colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (1. / 16.));
					colours[neighbourIndex].Clamp();
					colours[neighbourIndex].Update();
				}

				neighbourIndex = size_t(x + (y + 1) * imgWidth);
				colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (5. / 16.));
				colours[neighbourIndex].Clamp();
				colours[neighbourIndex].Update();
			}

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}
}

void Dither::NoDither(Image& image, const Palette& palette) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	// Create a copy of of image in Colour form
	const size_t coloursSize = (size_t)image.GetHeight() * image.GetWidth();
	std::vector<Colour> colours;
	colours.reserve(coloursSize);

	Log::StartTime();
	Log::WriteOneLine("NO DITHER...");

	Log::WriteOneLine("  Copying Pixels");
	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t index = image.GetIndex(x, y);
			//Colour pixel = GetColourFromImage(image, x, y);
			colours.emplace_back(GetColourFromImage(image, x, y));

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}

	Log::WriteOneLine("  Quantising");
	for (int x = 0; x < imgWidth; ++x) {
		for (int y = 0; y < imgHeight; ++y) {
			const size_t indexCol = size_t(x + y * imgWidth);
			const size_t index = image.GetIndex(x, y);
			Colour pixel = colours[indexCol];

			if (m_mono) pixel.ToGrayscale();

			SetColourMathMode(m_distanceMode);

			pixel = ClosestColour(pixel, palette, image.IsGrayscale());
			if (image.HasAlphaChannel() && m_ditherAlpha) DitherAlpha(pixel, colours, x, y, imgWidth, imgHeight);

			SetColourToImage(pixel, image, x, y);

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				const std::string maxStr = Log::ToString(2 * imgHeight * imgWidth);
				const std::string currStr = Log::ToString(x + y * imgWidth, static_cast<unsigned int>(maxStr.size()), ' ');

				Log::WriteOneLine("    " + currStr + " / " + maxStr);

				Log::StartTime();
			}
		}
	}
}

void Dither::SetSettings(
	const std::string distanceType,
	const std::string mathMode,
	const bool mono,
	const std::string matrixType,
	const bool ditherAlpha,
	const unsigned int ditherAlphaFactor,
	const std::string ditherAlphaType) {
	m_distanceMode = distanceType;
	m_mathMode = mathMode;
	m_mono = mono;
	m_matrixType = matrixType;
	m_ditherAlpha = ditherAlpha;
	m_ditherAlphaFactor = ditherAlphaFactor;
	m_ditherAlphaType = ditherAlphaType;
}

Colour Dither::ClosestColour(const Colour& col, const Palette& palette, const bool grayscale) {
	Colour closest = Colour::FromsRGB(0, 0, 0, 0);

	double min = 0., max = 1.;

	if (m_mono) {
		// get darkest and lightest colour in palette
		min = palette.GetColour(0).MonoGetLightness();
		max = min;
		for (size_t i = 1; i < palette.size(); ++i) {
			const double l = palette.GetColour(i).MonoGetLightness();
			min = l < min ? l : min;
			max = l > max ? l : max;
		}
	}

	size_t startI = 0;
	if (grayscale && !m_mono) {
		// if grayscale == true - first find earliest grayscale colour
		for (size_t i = 0; i < palette.size(); ++i) {
			++startI;
			if (palette.GetColour(i).IsGrayscale()) {
				closest = palette.GetColour(i);
				break;
			}
		}
	} else {
		closest = palette.GetColour(0);
		startI = 1;
	}

	if (palette.size() == startI) return closest;

	double closestDist = m_mono ? col.MonoDistance(closest, min, max) : col.MagSq(closest);

	for (size_t i = startI; i < palette.size(); ++i) {
		const Colour current = palette.GetColour(i);

		// When grayscale == true - only check grayscale colours
		// When grayscale == false - check all colours
		if ((!grayscale) || current.IsGrayscale()) {
			//double dist = col.MagSq(current);
			double dist = m_mono ? col.MonoDistance(current, min, max) : col.MagSq(current);

			if (dist < closestDist) {
				closestDist = dist;
				closest = current;
			}
		}
	}
	closest.SetAlpha(col.GetAlpha());

	return closest;
}

void Dither::DitherAlpha(Colour& col, std::vector<Colour>& colours, const int x, const int y, const int imgWidth, const int imgHeight) {
	// Skip fully opaque or fully transparent pixels
	if (col.GetAlpha() == 1. || col.GetAlpha() == 0) return;

	//const size_t indexCol = size_t(x + y * imgWidth);
	if (m_ditherAlphaType == "fs") {
		// Floyd-Steinberg Dither Alpha
		const double oldAlpha = col.GetAlpha();

		double newAlpha = std::floor(static_cast<double>(m_ditherAlphaFactor + 1) * oldAlpha) / static_cast<double>(m_ditherAlphaFactor);

		col.SetAlpha(newAlpha);

		double quantError = oldAlpha - newAlpha;

		if (x + 1 < imgWidth) {
			size_t neighbourIndex = size_t((x + 1) + y * imgWidth);
			double currAlpha = colours[neighbourIndex].GetAlpha() + (quantError * (7. / 16.));
			currAlpha = currAlpha > 1. ? 1. : (currAlpha < 0. ? 0. : currAlpha);
			colours[neighbourIndex].SetAlpha(currAlpha);
		}

		if (y + 1 < imgHeight) {
			size_t neighbourIndex = 0;
			double currAlpha = 0.;

			if (x - 1 >= 0) {
				neighbourIndex = size_t((x - 1) + (y + 1) * imgWidth);
				currAlpha = colours[neighbourIndex].GetAlpha() + (quantError * (3. / 16.));
				currAlpha = currAlpha > 1. ? 1. : (currAlpha < 0. ? 0. : currAlpha);
				colours[neighbourIndex].SetAlpha(currAlpha);
			}

			if (x + 1 < imgWidth) {
				neighbourIndex = size_t((x + 1) + (y + 1) * imgWidth);
				currAlpha = colours[neighbourIndex].GetAlpha() + (quantError * (1. / 16.));
				currAlpha = currAlpha > 1. ? 1. : (currAlpha < 0. ? 0. : currAlpha);
				colours[neighbourIndex].SetAlpha(currAlpha);
			}

			neighbourIndex = size_t(x + (y + 1) * imgWidth);
			currAlpha = colours[neighbourIndex].GetAlpha() + (quantError * (5. / 16.));
			currAlpha = currAlpha > 1. ? 1. : (currAlpha < 0. ? 0. : currAlpha);
			colours[neighbourIndex].SetAlpha(currAlpha);
		}
	} else if (m_ditherAlphaType == "ordered") {
		// Ordered Dither Alpha

		double newAlpha = col.GetAlpha();

		const double r = 1. / static_cast<double>(m_ditherAlphaFactor);
		const double M = Threshold::GetThreshold(x, y);

		newAlpha += M * r;
		newAlpha = newAlpha < 0. ? 0. : (newAlpha > 1. ? 1. : newAlpha);

		newAlpha = std::floor(static_cast<double>(m_ditherAlphaFactor + 1) * newAlpha) / static_cast<double>(m_ditherAlphaFactor);

		col.SetAlpha(newAlpha);
	} else {
		// No Dither Alpha
		double newAlpha = std::floor(static_cast<double>(m_ditherAlphaFactor + 1) * col.GetAlpha()) / static_cast<double>(m_ditherAlphaFactor);

		col.SetAlpha(newAlpha);
	}
}

// Fix for E0847: expression must have integral or enum type
// The error is caused by using a std::string in a switch statement.
// Replace the switch statement with if-else statements.

void Dither::SetColourMathMode(const std::string& mode) {
	if (mode == "srgb") {
		Colour::SetMathMode(Colour::MathMode::sRGB);
	} else if (mode == "oklab") {
		Colour::SetMathMode(Colour::MathMode::OkLab);
	} else if (mode == "oklab_l") {
		Colour::SetMathMode(Colour::MathMode::OkLab_Lightness);
	} else if (mode == "lrgb") {
		Colour::SetMathMode(Colour::MathMode::Linear_RGB);
	} else {
		Colour::SetMathMode(Colour::MathMode::sRGB);
	}
}
Colour Dither::GetColourFromImage(const Image& image, const int x, const int y) {
	const size_t index = image.GetIndex(x, y);
	if (image.IsGrayscale()) {
		const uint8_t data = image.GetData(index);
		uint8_t alpha = 255;

		if (image.GetChannels() == 2) {
			alpha = image.GetData(index + 1);
		}

		return Colour::FromsRGB(data, data, data, alpha);
	} else {
		uint8_t r, g, b, a;

		a = 255;
		if (image.GetChannels() == 4) {
			if (image.GetData(index + 3) > 0 && image.GetData(index + 3) < 255) {
				bool temp = true;
			}
			if (image.GetData(index + 3) == 0) {
				bool temp = true;
			}

			a = image.GetData(index + 3);
		}
		r = image.GetData(index + 0);
		g = image.GetData(index + 1);
		b = image.GetData(index + 2);

		return Colour::FromsRGB(r, g, b, a);
	}
}

void Dither::SetColourToImage(const Colour& colour, Image& image, const int x, const int y) {
	const size_t index = image.GetIndex(x, y);
	Colour::sRGB_UInt colour_int = colour.GetsRGB_UInt();

	if (image.IsGrayscale()) {
		if (image.GetChannels() == 2) {
			image.SetData(index, colour_int.r);
			image.SetData(index + 1, colour_int.a); // keep alpha channel
		} else {
			image.SetData(index, colour_int.r);
		}
	} else {
		if (image.GetChannels() == 4) {
			image.SetData(index + 0, colour_int.r);
			image.SetData(index + 1, colour_int.g);
			image.SetData(index + 2, colour_int.b);
			image.SetData(index + 3, colour_int.a);
		} else {
			image.SetData(index + 0, colour_int.r);
			image.SetData(index + 1, colour_int.g);
			image.SetData(index + 2, colour_int.b);
		}
	}
}

void Dither::ImageToGrayscale(Image& image) {
	// Convert image to grayscale
	SetColourMathMode(m_distanceMode);

	const int channels = image.GetChannels() == 3 ? 1 : 2;
	Image newImage(image.GetWidth(), image.GetHeight(), channels);

	for (int x = 0; x < image.GetWidth(); ++x) {
		for (int y = 0; y < image.GetHeight(); ++y) {
#ifdef _DEBUG
			const bool debug = x == 10 && y == 10;
			if (debug) {
				bool test = true;
			}
#endif // _DEBUG

			const size_t newIndex = newImage.GetIndex(x, y);
			Colour col = Dither::GetColourFromImage(image, x, y);
			const double l_d = col.MonoGetLightness();

			if (Colour::GetMathMode() == Colour::MathMode::sRGB) {
				col.SetsRGB_D(l_d, l_d, l_d);
			} else if (Colour::GetMathMode() == Colour::MathMode::Linear_RGB) {
				col.SetLRGB(l_d, l_d, l_d);
			} else {
				col.SetOkLab(l_d, 0., 0.);
			}

#ifdef _DEBUG
			if (debug) {
				const Colour::sRGB_UInt testVal = col.GetsRGB_UInt();

				bool test = true;
			}
#endif // _DEBUG

			newImage.SetData(newIndex, col.GetsRGB_UInt().r);

			if (channels == 2) {
				const size_t oldIndex = image.GetIndex(x, y) + 3;
				newImage.SetData(newIndex + 1, image.GetData(oldIndex));
			}
		}
	}

	image = newImage;
}