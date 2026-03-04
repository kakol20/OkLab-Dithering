#include "Threshold.h"

#include "../misc/Random.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <random>
#include <string>
#include <utility>
#include <vector>

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

	Random::Seed = 20260303;
	std::mt19937 rng(Random::Seed);

	m_blueNoise = std::vector<unsigned int>(size * size);
	for (size_t i = 0; i < static_cast<size_t>(size) * size; ++i) {
		m_blueNoise[i] = static_cast<unsigned int>(i);
		//m_blueNoise[i] = Random::RandUInt(0, (size * size) - 1);
	}

	std::shuffle(m_blueNoise.begin(), m_blueNoise.end(), rng);

	int maxIter = 40000;
	for (int i = 0; i < maxIter; ++i) {
		// select two random
		size_t ia = static_cast<size_t>(Random::RandUInt(0, (size * size) - 1));
		size_t ib = static_cast<size_t>(Random::RandUInt(0, (size * size) - 1));
		if (ia == ib) continue;

		unsigned int a = m_blueNoise[ia];
		unsigned int b = m_blueNoise[ib];

		// Find energy before and after swapping pixels.
		double startEnergy = BlueNoiseEnergy(a, (int)ia % size, (int)ia / size) + BlueNoiseEnergy(b, (int)ib % size, (int)ib / size);
		std::swap(m_blueNoise[ia], m_blueNoise[ib]);
		double endEnergy = BlueNoiseEnergy(a, (int)ia % size, (int)ia / size) + BlueNoiseEnergy(b, (int)ib % size, (int)ib / size);

		// If the energy was lower before the swap, then swap back.
		if (startEnergy < endEnergy) std::swap(m_blueNoise[ia], m_blueNoise[ib]);
	}
}

double Threshold::BlueNoiseEnergy(const unsigned int a, const int x, const int y) const {
	size_t ia = static_cast<size_t>(y) * m_blueNoiseSize + x;

	double total = 0.;
	for (int dy = -4; dy <= 4; ++dy) {
		for (int dx = -4; dx <= 4; ++dx) {
			if (dx == 0 && dy == 0) continue;

			int bx = (x + dx + m_blueNoiseSize) % m_blueNoiseSize;
			int by = (y + dy + m_blueNoiseSize) % m_blueNoiseSize;

			size_t ib = static_cast<size_t>(by) * m_blueNoiseSize + bx;
			unsigned int b = m_blueNoise[ib];

			// toroidal (wrapped) distance

			const double n = (m_blueNoiseSize * m_blueNoiseSize) - 1.;

			double ds = std::abs(static_cast<double>((int)b - (int)a));
			if (ds > n * 0.5) {
				ds = n - ds;
			}
			//ds /= (m_blueNoiseSize * m_blueNoiseSize) - 1.;

			double di = std::abs(static_cast<double>((int)ib - (int)ia));
			if (di > n * 0.5) {
				di = n - di;
			}
			//di /= (m_blueNoiseSize * m_blueNoiseSize) - 1.;

			double test = -((ds * ds) / (2.1 * 2.1)) - std::pow(di, 1. / 2.);

			total += std::exp(test);
		}
	}

	return total;
}