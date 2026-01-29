#pragma once
#include <vector>
#include <array>
#include <string>
//#include <cstdint>

class Threshold {
public:
	Threshold() {};
	~Threshold() {};

	static void GenerateThreshold(const std::string& matrixType);

	static double GetThreshold(const int x, const int y);

	static bool IsValidBayerSetting(const std::string& matrixType);

private:
	static int m_bayerSize;
	static std::array<unsigned int, 256> m_blueNoise16;
	static std::string m_matrixType;
	static std::vector<unsigned int> m_bayer;

	static inline size_t MatrixIndex(const int x, const int y, const int size) { return size_t(x + y * size); };

	static std::vector<unsigned int> GenerateBayerHalf(const int n);

	static bool IsPowerOfTwo(const int n) { return n > 0 && (n & (n - 1)) == 0; };
};
