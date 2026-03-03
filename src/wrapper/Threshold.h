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
	std::string m_matrixType = "bayer16";

	inline size_t MatrixIndex(const int x, const int y, const int size) const { return size_t(x + y * size); };

	std::vector<unsigned int> GenerateBayerHalf(const int n);

	static bool IsPowerOfTwo(const int n) { return n > 0 && (n & (n - 1)) == 0; };

	static bool IsValidBayerSetting(const std::string& matrixType);

	std::vector<unsigned int> m_bayer{ 0, 2, 3, 1 };

	std::array<uint8_t, 256> m_blueNoise16	{
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


	/// <summary>
	/// Based on Parker Square
	/// </summary>
	std::array<uint8_t, 9> m_parkerDither{
		29,  1, 47,
		41, 37,  1,
		23, 41, 29
	};
};
