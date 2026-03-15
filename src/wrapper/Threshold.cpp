#include "Threshold.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <vector>
//#include "../misc/Random.h"
//#include <cstdlib>
//#include <utility>

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
		out = static_cast<double>(m_parkerDither[MatrixIndex(x % 3, y % 3, 3)]) / 100.;
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
	// https://blog.demofox.org/2019/06/25/generating-blue-noise-textures-with-void-and-cluster/

	const int N = size * size;
	std::vector<double> noise = GenerateBlueNoiseField(size, 20260304);

	std::vector<int> idx(N);
	std::iota(idx.begin(), idx.end(), 0);

	// Rank pixels by noise value
	std::sort(idx.begin(), idx.end(),
		[&](int a, int b) {
			return noise[a] < noise[b];
		});

	// Build permutation
	m_blueNoiseSize = size;
	m_blueNoise = std::vector<unsigned int>(N);
	for (int i = 0; i < N; ++i) {
		m_blueNoise[idx[i]] = i;
	}
}

std::vector<double> Threshold::GenerateBlueNoiseField(const int n, const uint32_t seed) {
	const int N = n * n;
	std::vector<double> field(N);

	std::mt19937 rng(seed);
	std::uniform_real_distribution<double> dist(0., 1.);

	// Initial White Noise
	for (int i = 0; i < N; ++i)
		field[i] = dist(rng);

	// Repulsion iterations
	const int iterations = 10;
	const double sigma2 = (n * n) * 0.0025;

	for (int it = 0; it < iterations; ++it) {
		std::vector<double> next = field;

		for (int y = 0; y < n; ++y) {
			for (int x = 0; x < n; ++x) {
				int i = y * n + x;
				double force = 0.;

				for (int oy = -3; oy <= 3; ++oy) {
					for (int ox = -3; ox <= 3; ++ox) {
						if (ox == 0 && oy == 0) continue;

						int nx = Wrap(x + ox, n);
						int ny = Wrap(y + oy, n);
						int j = ny * n + nx;

						double d2 = ToroidalDist2(x, y, nx, ny, n);
						double w = std::exp(-d2 / sigma2);
						
						force += (field[i] - field[j]) * w;
					}
				}
				next[i] += 0.1 * force;
			}
		}
		field.swap(next);
	}

	return field;
}

inline double Threshold::ToroidalDist2(int x0, int y0, int x1, int y1, int n) {
	int dx = std::abs(x1 - x0);
	int dy = std::abs(y1 - y0);

	dx = std::min(dx, n - dx);
	dy = std::min(dy, n - dy);

	return static_cast<double>(dx * dx + dy * dy);
}
