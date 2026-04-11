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

	static void SetShape(const int width, const int height, const std::vector<std::vector<int>>& points);

	static bool IsValidSetting(const std::string& matrixType);

private:
	struct Shape {
		int width = 0, height = 0;
		std::vector<std::vector<int>> points;
	};

	//struct IVec2 { int x = 0, y = 0; };
	//struct Vec2 { 
	//	double x = 0., y = 0.; 
	//};

	int m_bayerSize = 2;
	int m_blueNoiseSize = 2;
	std::string m_matrixType = "bayer16";

	static Shape m_shape;

	inline size_t MatrixIndex(const int x, const int y, const int size) const { return size_t(x + y * size); };

	std::vector<unsigned int> GenerateBayerHalf(const int n);
	void GenerateBayerShape();

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

	/// <summary>
	/// Any size >= 2 that is a power of 2 is allowed
	/// </summary>
	/// <param name="matrixType"></param>
	static bool IsValidBayerShapeSetting(const std::string& matrixType);

	std::vector<unsigned int> m_bayer{ 0, 2, 3, 1 };
	std::vector<unsigned int> m_bayerShape{ 0, 2, 3, 1 };

	std::vector<uint32_t> m_blueNoise;

	/// <summary>
	/// Based on Parker Square
	/// </summary>
	static const std::array<uint8_t, 9> m_parkerDither;

	static const std::array<uint8_t, 54> m_heartDither;
	static const std::array<uint8_t, 225> m_circleDither;
};
