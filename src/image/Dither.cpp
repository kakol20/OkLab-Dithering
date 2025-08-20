#include "Dither.h"

#include "../wrapper/Log.h"

std::array<uint8_t, 256> Dither::m_bayer16{
		 0, 192,  48, 240,  12, 204,  60, 252,   3, 195,  51, 243,  15, 207,  63, 255,
	 128,  64, 176, 112, 140,  76, 188, 124, 131,  67, 179, 115, 143,  79, 191, 127,
		32, 224,  16, 208,  44, 236,  28, 220,  35, 227,  19, 211,  47, 239,  31, 223,
	 160,  96, 144,  80, 172, 108, 156,  92, 163,  99, 147,  83, 175, 111, 159,  95,
		 8, 200,  56, 248,   4, 196,  52, 244,  11, 203,  59, 251,   7, 199,  55, 247,
	 136,  72, 184, 120, 132,  68, 180, 116, 139,  75, 187, 123, 135,  71, 183, 119,
		40, 232,  24, 216,  36, 228,  20, 212,  43, 235,  27, 219,  39, 231,  23, 215,
	 168, 104, 152,  88, 164, 100, 148,  84, 171, 107, 155,  91, 167, 103, 151,  87,
		 2, 194,  50, 242,  14, 206,  62, 254,   1, 193,  49, 241,  13, 205,  61, 253,
	 130,  66, 178, 114, 142,  78, 190, 126, 129,  65, 177, 113, 141,  77, 189, 125,
		34, 226,  18, 210,  46, 238,  30, 222,  33, 225,  17, 209,  45, 237,  29, 221,
	 162,  98, 146,  82, 174, 110, 158,  94, 161,  97, 145,  81, 173, 109, 157,  93,
		10, 202,  58, 250,   6, 198,  54, 246,   9, 201,  57, 249,   5, 197,  53, 245,
	 138,  74, 186, 122, 134,  70, 182, 118, 137,  73, 185, 121, 133,  69, 181, 117,
		42, 234,  26, 218,  38, 230,  22, 214,  41, 233,  25, 217,  37, 229,  21, 213,
	 170, 106, 154,  90, 166, 102, 150,  86, 169, 105, 153,  89, 165, 101, 149,  85
};

void Dither::OrderedDither(Image& image, const Palette& palette, const std::string distanceType, const std::string mathMode) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	Log::StartTime();

	Log::WriteOneLine("Ordered Dither...");
	for (int x = 0; x < imgWidth; ++x) {
		for (int y = 0; y < imgHeight; ++y) {
			const size_t index = image.GetIndex(x, y);
			Colour pixel = Colour::FromsRGB(image.GetData(index + 0),
				image.GetData(index + 1),
				image.GetData(index + 2));

			// ===== APPLY DITHER =====
			if (mathMode == "oklab") {
				Colour::SetMathMode(Colour::MathMode::OkLab_Lightness);
			} else {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			}

			double threshold = (double)m_bayer16[MatrixIndex(x % 16, y % 16)];
			threshold = ((threshold + 0.5) / 256.) - 0.5;

			Colour threshold_c;
			if (Colour::GetMathMode() == Colour::MathMode::sRGB) {
				threshold_c = Colour::FromsRGB_D(threshold, threshold, threshold);
			} else {
				threshold_c = Colour::FromOkLab(threshold, threshold, threshold);
			}

			Colour dithered = pixel + (threshold_c * (1. / 16.));
			dithered.Clamp();
			dithered.Update();

			if (distanceType == "oklab") {
				Colour::SetMathMode(Colour::MathMode::OkLab);
			} else {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			}
			Colour nearest = ClosestColour(dithered, palette);
			Colour::sRGB_UInt nearest_int = nearest.GetsRGB_UInt();

			image.SetData(index + 0, nearest_int.r);
			image.SetData(index + 1, nearest_int.g);
			image.SetData(index + 2, nearest_int.b);

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

void Dither::FloydDither(Image& image, const Palette& palette, const std::string distanceType, const std::string mathMode) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	// Create a copy of of image in Colour form
	const size_t coloursSize = (size_t)image.GetHeight() * image.GetWidth();
	std::vector<Colour> colours;
	colours.reserve(coloursSize);

	Log::StartTime();
	Log::WriteOneLine("Floyd-Steinberg Dither...");

	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t index = image.GetIndex(x, y);
			Colour pixel = Colour::FromsRGB(image.GetData(index + 0),
				image.GetData(index + 1),
				image.GetData(index + 2));

			colours.push_back(pixel);

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				double progress = double(x + y * imgWidth) / double(2 * imgHeight * imgWidth);
				progress *= 100.;

				std::string outStr = Log::ToString(progress, 6);
				outStr = Log::LeadingCharacter(outStr, 9);

				Log::WriteOneLine("\t" + outStr + "%");

				Log::StartTime();
			}
		}
	}

	// Dither
	for (int y = 0; y < imgHeight; ++y) {
		for (int x = 0; x < imgWidth; ++x) {
			const size_t indexCol = size_t(x + y * imgWidth);
			const size_t index = image.GetIndex(x, y);

			Colour oldPixel = colours[indexCol];

			if (distanceType == "oklab") {
				Colour::SetMathMode(Colour::MathMode::OkLab);
			} else {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			}
			Colour newPixel = ClosestColour(oldPixel, palette);
			Colour::sRGB_UInt newPixel_int = newPixel.GetsRGB_UInt();

			image.SetData(index + 0, newPixel_int.r);
			image.SetData(index + 1, newPixel_int.g);
			image.SetData(index + 2, newPixel_int.b);

			if (mathMode == "oklab") {
				Colour::SetMathMode(Colour::MathMode::OkLab_Lightness);
			} else {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			}
			Colour quantError = oldPixel - newPixel;

			if (x + 1 < imgWidth) {
				const size_t neighbourIndex = size_t((x + 1) + y * imgWidth);
				colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (7. / 16.));
				colours[neighbourIndex].Clamp();
				colours[neighbourIndex].Update();
			}

			if (y + 1 < imgHeight) {
				if (x - 1 >= 0) {
					const size_t neighbourIndex = size_t((x - 1) + (y + 1) * imgWidth);
					colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (3. / 16.));
					colours[neighbourIndex].Clamp();
					colours[neighbourIndex].Update();
				}
				if (x + 1 < imgWidth) {
					const size_t neighbourIndex = size_t((x + 1) + (y + 1) * imgWidth);
					colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (1. / 16.));
					colours[neighbourIndex].Clamp();
					colours[neighbourIndex].Update();
				}

				const size_t neighbourIndex = size_t(x + (y + 1) * imgWidth);
				colours[neighbourIndex] = colours[neighbourIndex] + (quantError * (5. / 16.));
				colours[neighbourIndex].Clamp();
				colours[neighbourIndex].Update();
			}

			// -- Check Time --
			if (Log::CheckTimeSeconds(5.)) {
				double progress = double((x + y * imgWidth) + (imgHeight * imgWidth)) / double(2 * imgHeight * imgWidth);
				progress *= 100.;

				std::string outStr = Log::ToString(progress, 6);
				outStr = Log::LeadingCharacter(outStr, 9);

				Log::WriteOneLine("\t" + outStr + "%");

				Log::StartTime();
			}
		}
	}
}

void Dither::NoDither(Image& image, const Palette& palette, const std::string distanceType) {
	const int imgWidth = image.GetWidth();
	const int imgHeight = image.GetHeight();

	Log::StartTime();
	Log::WriteOneLine("No Dither...");
	for (int x = 0; x < imgWidth; ++x) {
		for (int y = 0; y < imgHeight; ++y) {
			const size_t index = image.GetIndex(x, y);
			Colour pixel = Colour::FromsRGB(image.GetData(index + 0),
				image.GetData(index + 1),
				image.GetData(index + 2));

			if (distanceType == "oklab") {
				Colour::SetMathMode(Colour::MathMode::OkLab);
			} else {
				Colour::SetMathMode(Colour::MathMode::sRGB);
			}

			pixel = ClosestColour(pixel, palette);
			Colour::sRGB_UInt pixel_int = pixel.GetsRGB_UInt();

			image.SetData(index + 0, pixel_int.r);
			image.SetData(index + 1, pixel_int.g);
			image.SetData(index + 2, pixel_int.b);

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

Colour Dither::ClosestColour(const Colour& col, const Palette& palette) {
	Colour closest = palette.GetIndex(0);

	if (palette.size() == 1) return closest;

	double closestDist = col.MagSq(closest);

	for (size_t i = 1; i < palette.size(); ++i) {
		const Colour current = palette.GetIndex(i);
		double dist = col.MagSq(current);

		if (dist < closestDist) {
			closestDist = dist;
			closest = current;
		}
	}

	return closest;
}