#include "Threshold.h"
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <cctype>

std::string Threshold::m_matrixType = "bayer16";
int Threshold::m_bayerSize = 16;

std::vector<unsigned int> Threshold::m_bayer{ 0, 2, 3, 1 };

std::array<unsigned int, 256> Threshold::m_blueNoise16{
	111,  53, 141, 160, 113, 194,  73, 176, 200,  53, 150,  94,  68,  42,  86, 252,
	 31,  99, 238, 221,  37, 250, 147,  26,  42, 105, 219, 168, 193, 137,  21, 165,
	124, 176,  80,  22,  67, 172, 122,  87, 211, 130, 247,  29, 115,  57, 228, 211,
	 45, 200, 150, 130, 188, 104,  56, 235, 159,  64,   6, 179,  78, 241, 146,  69,
		9, 244,  58,  91, 229,  13, 203,  33, 185, 101, 143, 205,  38,  92, 189, 106,
	222, 162, 114,  40, 213, 155, 138,  72, 245,  85, 225,  51, 125, 156,  24, 134,
	 84, 194,  27, 254,  77,  49, 177, 114,  20,  45, 167, 104, 253, 175, 210,  62,
	100, 179, 143, 121, 171,  97, 235, 128, 214, 148, 198,  17,  73,  32, 238,  48,
	231,  37,  70,  20, 204,  61,  25, 191,  88,  62, 111, 220, 139,  87, 119, 152,
	207, 129, 243, 159, 223, 109,  39, 247, 163,  30, 233, 182,  55, 197, 169,  15,
	107, 187,  54,  90, 136, 185, 152,  79,  51, 133,  98, 156,  40, 249,  95,  65,
	 24,  76, 218,  43,   0,  68, 227, 120, 195, 239,  11,  75, 126,  27, 226, 142,
	246, 173, 119, 199, 251, 103, 145,  21, 208, 172, 109, 217, 191,  83, 202, 162,
	 34,  93, 149,  28, 165, 181,  58,  36,  90,  66,  46, 139, 166,  59, 116,  49,
	215, 232,  63, 127,  82, 237, 216, 117, 158, 255, 184,  33, 242, 102,  12, 132,
	 75, 190,  17, 209,  47,  96,  15, 135, 230,  81,  19, 123, 224, 206, 154, 181
};

void Threshold::GenerateThreshold(const std::string& matrixType) {
	m_matrixType = matrixType;

	if (IsValidBayerSetting(m_matrixType)) {
		std::string numberPart = m_matrixType.substr(5);

		m_bayerSize = std::stoi(numberPart);
		m_bayer = GenerateBayerHalf(m_bayerSize);
	}
}

double Threshold::GetThreshold(const int x, const int y) {
	double out = 0.;
	if (IsValidBayerSetting(m_matrixType)) {
		out = static_cast<double>(m_bayer[MatrixIndex(x % m_bayerSize, y % m_bayerSize, m_bayerSize)]);
		out /= static_cast<double>(m_bayerSize * m_bayerSize);
	} else if (m_matrixType == "ign") {
		// https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
		out = std::fmod(52.9829189 * std::fmod(0.06711056 * double(x) + 0.00583715 * double(y), 1.), 1.);
	} else if (m_matrixType == "bluenoise16") {
		out = static_cast<double>(m_blueNoise16[MatrixIndex(x % 16, y % 16, 16)] / 256.);
	}

	return out - 0.5;
}

bool Threshold::IsValidBayerSetting(const std::string& matrixType) {
	// Must be at least "bayerN"
	if (matrixType.size() <= 5) return false;

	// First 5 chars must be exactly "bayer"
	if (matrixType.compare(0, 5, "bayer") != 0) return false;

	for (size_t i = 5; i < matrixType.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(matrixType[i])))
			return false;
	}

	const int size = std::stoi(matrixType.substr(5));

	return IsPowerOfTwo(size);
}

std::vector<unsigned int> Threshold::GenerateBayerHalf(const int n) {
	if (n == 2 || !IsPowerOfTwo(n)) return { 0, 2, 3, 1 };

	const int half = n / 2;
	auto prev = GenerateBayerHalf(half);

	std::vector<unsigned int> out(n * n);

	for (size_t x = 0; x < half; ++x) {
		for (size_t y = 0; y < half; ++y) {
			unsigned int v = prev[y * half + x];

			out[(y)*n + (x)] = 4 * v + 0;
			out[(y)*n + (x + half)] = 4 * v + 2;
			out[(y + half) * n + (x)] = 4 * v + 3;
			out[(y + half) * n + (x + half)] = 4 * v + 1;
		}
	}

	return out;
}