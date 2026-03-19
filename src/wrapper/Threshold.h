#pragma once
#include <vector>
#include <array>
#include <string>
#include <cstdint>
//#include <cstdint>

class Threshold {
public:
	Threshold() {};
	~Threshold() {};

	void GenerateThreshold(const std::string& matrixType);

	double GetThreshold(const int x, const int y) const;

	static bool IsValidSetting(const std::string& matrixType);

private:
	int m_bayerSize = 2;
	int m_blueNoiseSize = 2;
	std::string m_matrixType = "bayer16";

	inline size_t MatrixIndex(const int x, const int y, const int size) const { return size_t(x + y * size); };

	std::vector<unsigned int> GenerateBayerHalf(const int n);
	void GenerateBlueNoise(const int size);

	std::vector<double> GenerateBlueNoiseField(const int n, const uint32_t seed = 1);

	inline int Wrap(const int v, const int n) { return (v % n + n) % n; }

	inline double ToroidalDist2(int x0, int y0, int x1, int y1, int n);

	static bool IsPowerOfTwo(const int n) { return n > 0 && (n & (n - 1)) == 0; };

	/// <summary>
	/// Any size >= 2 that is a power of 2 is allowed
	/// </summary>
	/// <param name="matrixType"></param>
	/// <returns></returns>
	static bool IsValidBayerSetting(const std::string& matrixType);

	/// <summary>
	/// Any size >= 2 is allowed
	/// </summary>
	/// <param name="matrixType"></param>
	/// <returns></returns>
	static bool IsValidBlueNoiseSetting(const std::string& matrixType);

	std::vector<unsigned int> m_bayer{ 0, 2, 3, 1 };

	std::vector<unsigned int> m_blueNoise;

	/// <summary>
	/// Based on Parker Square
	/// </summary>
	static const std::array<uint8_t, 9> m_parkerDither;

	static const std::array<uint8_t, 54> m_heartDither;
	static const std::array<uint8_t, 225> m_circleDither;
};
