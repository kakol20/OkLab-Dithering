#pragma once
#include <cstdint>
#include <string>
class Colour {
public:
	Colour();
	Colour(const Colour& other);
	Colour(const double l, const double a, const double b, const double alpha = 1.);
	Colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);
	Colour(const char* hex);
	~Colour();

	Colour& operator=(const Colour& other);

	/// <summary>
	/// Choose update function based on mathMode
	/// </summary>
	/// <param name="mathMode"></param>
	void Update();

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
	/// Assign Colour via sRGB values
	/// </summary>
	/// <param name="r">0 to 1</param>
	/// <param name="g">0 to 1</param>
	/// <param name="b">0 to 1</param>
	/// <param name="a">0 to 1</param>
	/// <returns></returns>
	static Colour FromsRGB_D(const double r, const double g, const double b, const double a = 1.);

	/// <summary>
	/// Assign Colourvia OkLab values
	/// </summary>
	/// <param name="l"></param>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="alpha"></param>
	/// <returns></returns>
	static Colour FromOkLab(const double l, const double a, const double b, const double alpha = 1.);

	/// <summary>
	/// Get Colour via hex value
	/// </summary>
	/// <param name="hex"></param>
	/// <returns></returns>
	static Colour FromHex(const char* hex);

	// ========== ARITHMETIC ==========

	/// <summary>
	/// NOTE: Will not update other colour space when maths is done - must call UPDATE functions
	/// </summary>
	/// <param name="sRGB">Maths is treated like the values are 0 to 1</param>
	/// <param name="Oklab"></param>
	/// <param name="OkLab_Lightness">Only does maths on the lightness value</param>
	/// <param name="Linear_RGB"></param>
	enum class MathMode { sRGB, OkLab, OkLab_Lightness, Linear_RGB };

	/// <summary>
	/// <para>Clamp value based on m_mathMode</para>
	/// <para>NOTE: Will not update other colour space - must call UPDATE functions</para>
	/// </summary>
	void Clamp();

	Colour& operator/=(const Colour& other);
	Colour& operator*=(const Colour& other);
	Colour& operator+=(const Colour& other);
	Colour& operator-=(const Colour& other);
	Colour& operator*=(const double scalar);

	Colour operator/(const Colour& other) const;
	Colour operator*(const Colour& other) const;
	Colour operator+(const Colour& other) const;
	Colour operator-(const Colour& other) const;
	Colour operator*(const double scalar) const;

	bool operator==(const Colour& other) const;
	bool operator<(const Colour& other) const;
	
	inline bool operator!=(const Colour& other) const { return !(*this == other); };
	inline bool operator>(const Colour& other) const { return other < *this; };
	inline bool operator<=(const Colour& other) const { return !(*this > other); };
	inline bool operator>=(const Colour& other) const { return !(*this < other); };

	/// <summary>
	/// NOTE: Will not update other colour space when maths is done - must call UPDATE functions
	/// </summary>
	/// <param name="sRGB">Maths is treated like the values are 0 to 1</param>
	/// <param name="Oklab"></param>
	/// <param name="OkLab_Lightness">Only does maths on the lightness value</param>
	static void SetMathMode(MathMode mode) { m_mathMode = mode; };
	static MathMode GetMathMode() { return m_mathMode; };

	struct sRGB { double r, g, b; };
	struct sRGB_UInt { uint8_t r, g, b, a; };
	struct OkLab { double l, a, b; };

	/// <summary>
	/// Linear RGB
	/// </summary>
	struct LRGB { double r, g, b; };

	// ========== DEBUGGING ==========

	std::string LRGBDebug() const;
	std::string OkLabDebug() const;
	std::string sRGBUintDebug() const;

	// ========== OTHER ==========

	/// <summary>
	/// <para>The squared distance between colours</para>
	/// </summary>
	/// <param name="other"></param>
	/// <returns>
	/// <para>NOTE: if MathMode == OkLab_Lightness - the absoulute difference</para>
	/// <para>between L values will be outputted instead</para>
	/// </returns>
	double MagSq(const Colour& other) const;

	/// <summary>
	/// Const member function that returns the squared length (squared magnitude) of the object.
	/// </summary>
	/// <returns>The squared length as a double.</returns>
	double LengthSq() const;

	double Dot(const Colour& other) const;

	/// <summary>
	/// Calculates distance based on lightness
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	double MonoDistance(const Colour& other, const double min = 0., const double max = 1.) const;

	double MonoGetLightness() const;

	void ToGrayscale();

	void Abs();
	static Colour Min(const Colour& a, const Colour& b);
	void PureBlack(const uint8_t alpha = 255);

private:
	LRGB m_lrgb;
	OkLab m_oklab;
	sRGB m_srgb;

	double m_alpha;

	bool m_isGrayscale;

	//static OkLab sRGBtoOkLab(const sRGB val);

	static MathMode m_mathMode;

	void OkLabFallback();

	// Convert sRGB to Linear RGB
	void sRGBtoLRGB();
	// Convert Linear RGB to sRGB
	void LRGBtosRGB();

	// Convert Linear RGB to OkLab
	void LRGBtoOkLab();
	// Convert OkLab to Linear RGB
	void OkLabtoLRGB();

public:
	sRGB GetsRGB() const { return m_srgb; };
	OkLab GetOkLab() const { return m_oklab; };
	sRGB_UInt GetsRGB_UInt() const;

	double GetAlpha() const { return m_alpha; };
	void SetAlpha(const double alpha) { m_alpha = alpha; };

	bool IsGrayscale() const { return m_isGrayscale; };

	void SetHex(const char* hex);
	void SetLRGB(const double r, const double g, const double b, const double a = 1.);
	void SetOkLab(const double l, const double a, const double b, const double alpha = 1.);
	void SetsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);
	void SetsRGB_D(const double r, const double g, const double b, const double a = 1.);
};
