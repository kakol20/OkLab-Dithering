#include "Threshold.h"
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <cctype>
#include <random>
#include <algorithm>

void Threshold::GenerateThreshold(const std::string& matrixType) {
	m_matrixType = matrixType;

	if (IsValidBayerSetting(m_matrixType)) {
		std::string numberPart = m_matrixType.substr(5);

		m_bayerSize = std::stoi(numberPart);
		m_bayer = GenerateBayerHalf(m_bayerSize);
	} else if (IsValidBlueNoiseSetting(m_matrixType)) {
		std::string numberPart = m_matrixType.substr(9);

		m_blueNoiseSize = std::stoi(numberPart);
		GenerateBlueNoise(m_blueNoiseSize);
	}
}

double Threshold::GetThreshold(const int x, const int y) const {
	double out = 0.;
	if (IsValidBayerSetting(m_matrixType)) {
		out = static_cast<double>(m_bayer[MatrixIndex(x % m_bayerSize, y % m_bayerSize, m_bayerSize)]);
		out /= static_cast<double>(m_bayerSize * m_bayerSize);
	} else if (IsValidBlueNoiseSetting(m_matrixType)) {
		out = static_cast<double>(m_blueNoise[MatrixIndex(x % m_blueNoiseSize, y % m_blueNoiseSize, m_blueNoiseSize)]);
		out /= static_cast<double>(m_blueNoiseSize * m_blueNoiseSize);
	} else if (m_matrixType == "ign") {
		// https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
		out = std::fmod(52.9829189 * std::fmod(0.06711056 * double(x) + 0.00583715 * double(y), 1.), 1.);
	} else if (m_matrixType == "parkerdither") {
		out = static_cast<double>(m_parkerDither[MatrixIndex(x % 3, y % 3, 3)]) / 50.;
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

	if (size < 2) return false;

	return IsPowerOfTwo(size);
}

bool Threshold::IsValidBlueNoiseSetting(const std::string& matrixType) {
	// Must be at least "bluenoiseN"
	if (matrixType.size() <= 9) return false;

	// First 9 chars must be exactly "bluenoise"
	if (matrixType.compare(0, 9, "bluenoise") != 0) return false;

	for (size_t i = 9; i < matrixType.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(matrixType[i])))
			return false;
	}

	const int size = std::stoi(matrixType.substr(9));

	if (size < 2) return false;
	return true;
}

bool Threshold::IsValidSetting(const std::string& matrixType) {
	if (IsValidBayerSetting(matrixType)) return true;
	if (IsValidBlueNoiseSetting(matrixType)) return true;

	// settings["matrixType"] != "bluenoise16" && settings["matrixType"] != "ign"

	if (matrixType == "ign") return true;
	if (matrixType == "parkerdither") return true;
	//if (matrixType == "bluenoise16") return true;

	return false;
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

void Threshold::GenerateBlueNoise(const int size) {
	// https://abau.io/blog/blue_noise_dithering/
	m_blueNoise = std::vector<unsigned int>(size * size);
	for (size_t i = 0; i < static_cast<size_t>(size) * size; ++i) m_blueNoise[i] = static_cast<unsigned int>(i);

	std::mt19937 rng(20260303);
	std::shuffle(m_blueNoise.begin(), m_blueNoise.end(), rng);
}

double Threshold::BlueNoiseEnergy(std::vector<unsigned int>& grid, const unsigned int a, const int x, const int y) const {
	size_t ia = static_cast<size_t>(y) * m_blueNoiseSize + x;

	return 0.0;
}
