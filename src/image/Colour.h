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
	/// <param name="r">0 to 1</param>
	/// <param name="g">0 to 1</param>
	/// <param name="b">0 to 1</param>
	/// <param name="a">0 to 1</param>
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

	// ========== ARITHMETIC ==========

	/// <summary>
	/// <param name="sRGB">Maths is treated like the values are 0 to 1</param>
	/// <param name="Oklab"></param>
	/// <param name="OkLab_Lightness">Only does maths on the lightness value</param>
	/// </summary>
	enum class MathMode { sRGB, OkLab, OkLab_Lightness };

	Colour& operator/=(const Colour& other);
	Colour& operator*=(const Colour& other);
	Colour& operator+=(const Colour& other);
	Colour& operator-=(const Colour& other);
	Colour& operator*=(const double scalar);

private:
	struct sRGB {
		double r;
		double g;
		double b;
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

	static MathMode m_mathMode;

public:
	sRGB GetsRGB() const { return m_srgb; };
	OkLab GetOkLab() const { return m_oklab; };

	void SetsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);
	void SetOkLab(const double l, const double a, const double b, const double alpha = 1.);
};

