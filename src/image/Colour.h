#pragma once
#include <cstdint>
class Colour {
public:
	Colour();
	Colour(const Colour& other);
	~Colour();

	Colour& operator=(const Colour& other);

	/// <summary>
	/// Update OkLab from sRGB
	/// </summary>
	void UpdateOkLab();

	/// <summary>
	/// Update sRGB from OkLab
	/// </summary>
	void UpdatesRGB();

	/// <summary>
	/// Assign Colour based on sRGB values
	/// </summary>
	/// <param name="r">0 to 255</param>
	/// <param name="g">0 to 255</param>
	/// <param name="b">0 to 255</param>
	/// <param name="a">0 to 255</param>
	/// <returns></returns>
	static Colour FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);

	/// <summary>
	/// Assign Colour based on OkLab values
	/// </summary>
	/// <param name="l"></param>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="alpha"></param>
	/// <returns></returns>
	static Colour FromOkLab(const double l, const double a, const double b, const double alpha = 1.);

private:
	struct sRGB {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
	sRGB m_srgb;

	struct OkLab {
		double l;
		double a;
		double b;
	};
	OkLab m_oklab;

	double m_alpha;

	//static OkLab sRGBtoOkLab(const sRGB val);
};

