#include "Threshold.h"

#include "../misc/Random.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <limits>
#include "../wrapper/Log.h"

const std::array<uint8_t, 9> Threshold::m_parkerDither{
		29,  1, 47,
		41, 37,  1,
		23, 41, 29
};

const std::array<uint8_t, 54> Threshold::m_heartDither{
	2, 0, 2, 1, 2, 1, 0, 1, 0,
	2, 2, 2, 1, 1, 1, 0, 0, 0,
	1, 2, 1, 0, 1, 0, 2, 0, 2,
	1, 1, 1, 0, 0, 0, 2, 2, 2,
	0, 1, 0, 2, 0, 2, 1, 2, 1,
	0, 0, 0, 2, 2, 2, 1, 1, 1
};

const std::array<uint8_t, 225> Threshold::m_circleDither{
	10, 9, 9, 8, 8, 7, 7, 7, 7, 7, 8, 8, 9, 9, 10,
	 9, 8, 8, 7, 7, 6, 6, 6, 6, 6, 7, 7, 8, 8,  9,
	 9, 8, 7, 6, 6, 5, 5, 5, 5, 5, 6, 6, 7, 8,  9,
	 8, 7, 6, 6, 5, 4, 4, 4, 4, 4, 5, 6, 6, 7,  8,
	 8, 7, 6, 5, 4, 4, 3, 3, 3, 4, 4, 5, 6, 7,  8,
	 7, 6, 5, 4, 4, 3, 2, 2, 2, 3, 4, 4, 5, 6,  7,
	 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3, 4, 5, 6,  7,
	 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6,  7,
	 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3, 4, 5, 6,  7,
	 7, 6, 5, 4, 4, 3, 2, 2, 2, 3, 4, 4, 5, 6,  7,
	 8, 7, 6, 5, 4, 4, 3, 3, 3, 4, 4, 5, 6, 7,  8,
	 8, 7, 6, 6, 5, 4, 4, 4, 4, 4, 5, 6, 6, 7,  8,
	 9, 8, 7, 6, 6, 5, 5, 5, 5, 5, 6, 6, 7, 8,  9,
	 9, 8, 8, 7, 7, 6, 6, 6, 6, 6, 7, 7, 8, 8,  9,
	10, 9, 9, 8, 8, 7, 7, 7, 7, 7, 8, 8, 9, 9, 10
};

Threshold::Shape Threshold::m_shape{
	0, 0, std::vector<std::vector<int>>()
};

void Threshold::GenerateThreshold(const std::string& matrixType) {
	m_matrixType = matrixType;

	if (IsValidBayerSetting(m_matrixType)) {
		Log::WriteOneLine("Generating Threshold Map");
		std::string numberPart = m_matrixType.substr(5);

		m_bayerSize = std::stoi(numberPart);
		m_bayer = GenerateBayerHalf(m_bayerSize);
	} else if (IsValidBlueNoiseSetting(m_matrixType)) {
		Log::WriteOneLine("Generating Threshold Map");
		std::string numberPart = m_matrixType.substr(9);

		m_blueNoiseSize = std::stoi(numberPart);
		GenerateBlueNoise(m_blueNoiseSize);
	} else if (IsValidBayerShapeSetting(m_matrixType)) {
		Log::WriteOneLine("Generating Threshold Map");
		std::string numberPart = m_matrixType.substr(10);

		m_bayerSize = std::stoi(numberPart);
		m_bayer = GenerateBayerHalf(m_bayerSize);

		GenerateBayerShape();
	}
}

double Threshold::GetThreshold(const int x, const int y) const {
	double out = 0.5;
	if (IsValidBayerSetting(m_matrixType)) {
		out = static_cast<double>(m_bayer[MatrixIndex(x % m_bayerSize, y % m_bayerSize, m_bayerSize)]) + 1.;
		out /= static_cast<double>(m_bayerSize * m_bayerSize) + 1.;
	} else if (IsValidBlueNoiseSetting(m_matrixType)) {
		out = static_cast<double>(m_blueNoise[MatrixIndex(x % m_blueNoiseSize, y % m_blueNoiseSize, m_blueNoiseSize)]) + 1.;
		out /= static_cast<double>(m_blueNoiseSize * m_blueNoiseSize) + 1.;
	} else if (m_matrixType == "ign") {
		// https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
		out = std::fmod(52.9829189 * std::fmod(0.06711056 * double(x) + 0.00583715 * double(y), 1.), 1.);
	} else if (m_matrixType == "parkerdither") {
		out = static_cast<double>(m_parkerDither[MatrixIndex(x % 3, y % 3, 3)]) / 100.;
	} else if (m_matrixType == "heart") {
		out = static_cast<double>(m_heartDither[static_cast<size_t>((x % 9) + (y % 6) * 9)]) + 1.;
		out /= 2. + 2.;
	} else if (m_matrixType == "circle") {
		out = static_cast<double>(m_circleDither[MatrixIndex(x % 15, y % 15, 15)]) + 1.;
		out /= 10. + 2.;
	} else if (IsValidBayerShapeSetting(m_matrixType)) {
		const int width = m_bayerSize * m_shape.width;
		const int height = m_bayerSize * m_shape.height;

		out = static_cast<double>(m_bayerShape[static_cast<size_t>((x % width) + (y % height) * width)]) + 1.;
		out /= static_cast<double>(m_bayerSize * m_bayerSize) + 1.;
	}

	return out - 0.5;
}

void Threshold::SetShape(const int width, const int height, const std::vector<std::vector<int>>& points) {
	m_shape = {
		width, height,
		points
	};
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

bool Threshold::IsValidBayerShapeSetting(const std::string& matrixType) {
	// Must be at least "bayershapeN"
	if (matrixType.size() <= 10) return false;

	// First 5 chars must be exactly "bayer"
	if (matrixType.compare(0, 10, "bayershape") != 0) return false;

	for (size_t i = 10; i < matrixType.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(matrixType[i])))
			return false;
	}

	const int size = std::stoi(matrixType.substr(10));

	if (size < 2) return false;

	return IsPowerOfTwo(size);
}

bool Threshold::IsValidSetting(const std::string& matrixType) {
	if (matrixType == "ign") return true;
	if (matrixType == "parkerdither") return true;
	if (matrixType == "heart") return true;
	if (matrixType == "circle") return true;

	if (IsValidBayerSetting(matrixType)) return true;
	if (IsValidBlueNoiseSetting(matrixType)) return true;
	if (IsValidBayerShapeSetting(matrixType)) return true;

	// settings["matrixType"] != "bluenoise16" && settings["matrixType"] != "ign"
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

void Threshold::GenerateBayerShape() {
	const int width = m_bayerSize * m_shape.width;
	const int height = m_bayerSize * m_shape.height;

	m_bayerShape.clear();
	m_bayerShape.resize(static_cast<size_t>(width) * height);

	for (int xi = 0; xi < m_bayerSize; ++xi) {
		for (int yi = 0; yi < m_bayerSize; ++yi) {
			const size_t bayerIndex = MatrixIndex(xi, yi, m_bayerSize);
			const unsigned int value = m_bayer[bayerIndex];

			// set origin
			const int xo = xi * m_shape.width;
			const int yo = yi * m_shape.height;

			for (size_t i = 0; i < m_shape.points.size(); ++i) {
				// set position and wrap
				int x = xo + m_shape.points[i][0];
				int y = yo + m_shape.points[i][1];

				x = x % width;
				y = y % height;

				x = x < 0 ? x + width : x;
				y = y < 0 ? y + height : y;

				const size_t shapeIndex = static_cast<size_t>(x + y * width);

				m_bayerShape[shapeIndex] = value;
			}
		}
	}
}

void Threshold::GenerateBlueNoise(const int size) {
	// https://abau.io/blog/blue_noise_dithering/
	struct Point {
		int x, y;
	};

	m_blueNoiseSize = size;

	const int m = 5;
	//const int K = 16;

	m_blueNoise = std::vector<unsigned int>(size * size, 0);

	std::vector<Point> points;
	points.reserve(static_cast<size_t>(size * size));

	std::mt19937 rng(Random::Seed);
	std::uniform_int_distribution<int> dist(0, size - 1);

	Log::StartTime();
	for (int i = 0; i < size * size; ++i) {
		Point bestCandidate{};

		double bestDist = -INFINITY;

#ifdef _DEBUG
		int K = size > 16 ? (int)points.size() * 1 + 1 : (int)points.size() * m + 1;
		K = size >= 64 ? 16 : K;
#else
		const int K = size > 64 ? 16 : (int)points.size() * m + 1;
#endif // _DEBUG

		for (int k = 0; k < K; ++k) {
			Point candidate{ dist(rng), dist(rng) };

			double minDist = INFINITY;

			// Find distance to closest existing point
			for (const Point& p : points) {
				double d = std::sqrt(ToroidalDist2(candidate.x, candidate.y, p.x, p.y, size));
				if (d < minDist) {
					minDist = d;
				}
			}

			// If first point, accept immediately
			if (points.empty()) {
				bestCandidate = candidate;
				break;
			}

			if (minDist > bestDist) {
				bestDist = minDist;
				bestCandidate = candidate;
			}
		}

		points.push_back(bestCandidate);

		// Assign rank (normalized 0 → 1)
		int index = bestCandidate.y * size + bestCandidate.x;
		m_blueNoise[index] = i;

		Log::DebugProgress(double(i), double(size* size - 1), 5);
	}
}

inline double Threshold::ToroidalDist2(int x0, int y0, int x1, int y1, int n) {
	int dx = std::abs(x1 - x0);
	int dy = std::abs(y1 - y0);

	dx = std::min(dx, n - dx);
	dy = std::min(dy, n - dy);

	return static_cast<double>(dx * dx + dy * dy);
}