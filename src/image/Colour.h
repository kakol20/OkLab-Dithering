#pragma once
#include <cstdint>
class Colour {
public:
	Colour();
	Colour(const Colour& other);
	~Colour();

	Colour& operator=(const Colour& other);

	/// <summary>
	/// Assign Colour based on sRGB values
	/// </summary>
	/// <param name="r">0 to 255</param>
	/// <param name="g">0 to 255</param>
	/// <param name="b">0 to 255</param>
	/// <param name="a">0 to 255</param>
	/// <returns></returns>
	static Colour FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);

private:
	struct sRGB {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
	sRGB m_srgb;

	double m_alpha;
};

