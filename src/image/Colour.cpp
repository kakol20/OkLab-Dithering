#include "../wrapper/Log.h"
#include "../wrapper/Maths.hpp"
#include "Colour.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>

Colour::MathMode Colour::m_mathMode = Colour::MathMode::OkLab_Lightness;

Colour::Colour() {
	m_alpha = 1.;
	m_isGrayscale = false;
	m_lrgb = { 0., 0., 0. };
	m_oklab = { 0., 0., 0. };
	m_srgb = { 0, 0, 0 };
}

Colour::Colour(const Colour& other) {
	m_alpha = other.m_alpha;
	m_isGrayscale = other.m_isGrayscale;
	m_lrgb = other.m_lrgb;
	m_oklab = other.m_oklab;
	m_srgb = other.m_srgb;
}

Colour::Colour(const double l, const double a, const double b, const double alpha) {
	SetOkLab(l, a, b, alpha);
}

Colour::Colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	SetsRGB(r, g, b, a);
}

Colour::Colour(const char* hex) {
	SetHex(hex);
}

Colour::~Colour() {
}

Colour& Colour::operator=(const Colour& other) {
	if (this == &other) return *this;
	m_alpha = other.m_alpha;
	m_isGrayscale = other.m_isGrayscale;
	m_lrgb = other.m_lrgb;
	m_oklab = other.m_oklab;
	m_srgb = other.m_srgb;
	return *this;
}

void Colour::Update() {
	if (m_mathMode == MathMode::sRGB) {
		sRGBtoLRGB();
		LRGBtoOkLab();
	} else if (m_mathMode == MathMode::Linear_RGB) {
		LRGBtosRGB();
		LRGBtoOkLab();
	} else {
		OkLabtoLRGB();
		LRGBtosRGB();
	}
}

Colour Colour::FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Colour out(r, g, b, a);
	return out;
}

Colour Colour::FromsRGB_D(const double r, const double g, const double b, const double a) {
	Colour out;
	out.SetsRGB_D(r, g, b);
	return out;
}

Colour Colour::FromOkLab(const double l, const double a, const double b, const double alpha) {
	Colour out(l, a, b);
	return out;
}

Colour Colour::FromHex(const char* hex) {
	Colour out(hex);
	return out;
}

void Colour::Clamp() {
	if (m_mathMode == MathMode::sRGB) {
		m_srgb.r = m_srgb.r > 1. ? 1. : m_srgb.r;
		m_srgb.r = m_srgb.r < 0. ? 0. : m_srgb.r;

		m_srgb.g = m_srgb.g > 1. ? 1. : m_srgb.g;
		m_srgb.g = m_srgb.g < 0. ? 0. : m_srgb.g;

		m_srgb.b = m_srgb.b > 1. ? 1. : m_srgb.b;
		m_srgb.b = m_srgb.b < 0. ? 0. : m_srgb.b;
	} else if (m_mathMode == MathMode::Linear_RGB) {
		m_lrgb.r = m_lrgb.r > 1. ? 1. : m_lrgb.r;
		m_lrgb.r = m_lrgb.r < 0. ? 0. : m_lrgb.r;

		m_lrgb.g = m_lrgb.g > 1. ? 1. : m_lrgb.g;
		m_lrgb.g = m_lrgb.g < 0. ? 0. : m_lrgb.g;

		m_lrgb.b = m_lrgb.b > 1. ? 1. : m_lrgb.b;
		m_lrgb.b = m_lrgb.b < 0. ? 0. : m_lrgb.b;
	} else {
		m_oklab.l = m_oklab.l > 1. ? 1. : m_oklab.l;
		m_oklab.l = m_oklab.l < 0. ? 0. : m_oklab.l;
		OkLabFallback();
	}
}

Colour& Colour::operator/=(const Colour& other) {
	switch (m_mathMode) {
	case MathMode::sRGB:
		m_srgb.r /= other.m_srgb.r;
		m_srgb.g /= other.m_srgb.g;
		m_srgb.b /= other.m_srgb.b;
		break;
	case MathMode::OkLab:
		m_oklab.l /= other.m_oklab.l;
		m_oklab.a /= other.m_oklab.a;
		m_oklab.b /= other.m_oklab.b;
		break;
	case MathMode::OkLab_Lightness:
		m_oklab.l /= other.m_oklab.l;
		break;
	case MathMode::Linear_RGB:
		m_lrgb.r /= other.m_lrgb.r;
		m_lrgb.g /= other.m_lrgb.g;
		m_lrgb.b /= other.m_lrgb.b;
		break;
	}
	return *this;
}

Colour& Colour::operator*=(const Colour& other) {
	switch (m_mathMode) {
	case MathMode::sRGB:
		m_srgb.r *= other.m_srgb.r;
		m_srgb.g *= other.m_srgb.g;
		m_srgb.b *= other.m_srgb.b;
		break;
	case MathMode::OkLab:
		m_oklab.l *= other.m_oklab.l;
		m_oklab.a *= other.m_oklab.a;
		m_oklab.b *= other.m_oklab.b;
		break;
	case MathMode::OkLab_Lightness:
		m_oklab.l *= other.m_oklab.l;
		break;
	case MathMode::Linear_RGB:
		m_lrgb.r *= other.m_lrgb.r;
		m_lrgb.g *= other.m_lrgb.g;
		m_lrgb.b *= other.m_lrgb.b;
		break;
	}
	return *this;
}

Colour& Colour::operator+=(const Colour& other) {
	switch (m_mathMode) {
	case MathMode::sRGB:
		m_srgb.r += other.m_srgb.r;
		m_srgb.g += other.m_srgb.g;
		m_srgb.b += other.m_srgb.b;
		break;
	case MathMode::OkLab:
		m_oklab.l += other.m_oklab.l;
		m_oklab.a += other.m_oklab.a;
		m_oklab.b += other.m_oklab.b;
		break;
	case MathMode::OkLab_Lightness:
		m_oklab.l += other.m_oklab.l;
		break;
	case MathMode::Linear_RGB:
		m_lrgb.r += other.m_lrgb.r;
		m_lrgb.g += other.m_lrgb.g;
		m_lrgb.b += other.m_lrgb.b;
		break;
	}
	return *this;
}

Colour& Colour::operator-=(const Colour& other) {
	switch (m_mathMode) {
	case MathMode::sRGB:
		m_srgb.r -= other.m_srgb.r;
		m_srgb.g -= other.m_srgb.g;
		m_srgb.b -= other.m_srgb.b;
		break;
	case MathMode::OkLab:
		m_oklab.l -= other.m_oklab.l;
		m_oklab.a -= other.m_oklab.a;
		m_oklab.b -= other.m_oklab.b;
		break;
	case MathMode::OkLab_Lightness:
		m_oklab.l -= other.m_oklab.l;
		break;
	case MathMode::Linear_RGB:
		m_lrgb.r -= other.m_lrgb.r;
		m_lrgb.g -= other.m_lrgb.g;
		m_lrgb.b -= other.m_lrgb.b;
		break;
	}
	return *this;
}

Colour& Colour::operator*=(const double scalar) {
	switch (m_mathMode) {
	case MathMode::sRGB:
		m_srgb.r *= scalar;
		m_srgb.g *= scalar;
		m_srgb.b *= scalar;
		break;
	case MathMode::OkLab:
		m_oklab.l *= scalar;
		m_oklab.a *= scalar;
		m_oklab.b *= scalar;
		break;
	case MathMode::OkLab_Lightness:
		m_oklab.l *= scalar;
		break;
	case MathMode::Linear_RGB:
		m_lrgb.r *= scalar;
		m_lrgb.g *= scalar;
		m_lrgb.b *= scalar;
		break;
	}
	return *this;
}

Colour Colour::operator/(const Colour& other) const {
	Colour out(*this);
	out /= other;
	return out;
}

Colour Colour::operator*(const Colour& other) const {
	Colour out(*this);
	out *= other;
	return out;
}

Colour Colour::operator+(const Colour& other) const {
	Colour out(*this);
	out += other;
	return out;
}

Colour Colour::operator-(const Colour& other) const {
	Colour out(*this);
	out -= other;
	return out;
}

Colour Colour::operator*(const double scalar) const {
	Colour out(*this);
	out *= scalar;
	return out;
}

std::string Colour::LRGBDebug() const {
	std::string rStr = Log::LeadingCharacter(Log::ToString(m_lrgb.r, 4), 7, ' ');
	std::string gStr = Log::LeadingCharacter(Log::ToString(m_lrgb.g, 4), 7, ' ');
	std::string bStr = Log::LeadingCharacter(Log::ToString(m_lrgb.b, 4), 7, ' ');
	return rStr + ' ' + gStr + ' ' + bStr;
}

std::string Colour::OkLabDebug() const {
	std::string lStr = Log::LeadingCharacter(Log::ToString(m_oklab.l, 4), 7, ' ');
	std::string aStr = Log::LeadingCharacter(Log::ToString(m_oklab.a, 4), 7, ' ');
	std::string bStr = Log::LeadingCharacter(Log::ToString(m_oklab.b, 4), 7, ' ');
	return lStr + ' ' + aStr + ' ' + bStr;
}

std::string Colour::sRGBUintDebug() const {
	sRGB_UInt srgb_int = GetsRGB_UInt();
	return Log::ToString((unsigned int)srgb_int.r, 3, ' ') + ' ' +
		Log::ToString((unsigned int)srgb_int.g, 3, ' ') + ' ' +
		Log::ToString((unsigned int)srgb_int.b, 3, ' ');
}

double Colour::MagSq(const Colour& other) const {
	switch (m_mathMode) {
	case MathMode::sRGB:
		return Maths::Pow2(m_srgb.r - other.m_srgb.r) +
			Maths::Pow2(m_srgb.g - other.m_srgb.g) +
			Maths::Pow2(m_srgb.b - other.m_srgb.b);
	case MathMode::OkLab:
		return Maths::Pow2(m_oklab.l - other.m_oklab.l) +
			Maths::Pow2(m_oklab.a - other.m_oklab.a) +
			Maths::Pow2(m_oklab.b - other.m_oklab.b);
	case MathMode::OkLab_Lightness:
		return std::abs(m_oklab.l - other.m_oklab.l);
	case MathMode::Linear_RGB:
		return Maths::Pow2(m_lrgb.r - other.m_lrgb.r) +
			Maths::Pow2(m_lrgb.g - other.m_lrgb.g) +
			Maths::Pow2(m_lrgb.b - other.m_lrgb.b);
	}
	return 0.0;
}

double Colour::MonoDistance(const Colour& other, const double min, const double max) const {
	// Normalise other colour
	double otherL = (other.MonoGetLightness() - min) / (max - min);
	return std::abs(MonoGetLightness() - otherL);
}

double Colour::MonoGetLightness() const {
	if (m_mathMode == MathMode::sRGB) {
		return 0.2126 * m_srgb.r + 0.7152 * m_srgb.g + 0.0722 * m_srgb.b;
	} else if (m_mathMode == MathMode::Linear_RGB) {
		return 0.2126 * m_lrgb.r + 0.7152 * m_lrgb.g + 0.0722 * m_lrgb.b;
	} else {
		return m_oklab.l;
	}
}

void Colour::ToGrayscale() {
	if (m_mathMode == MathMode::sRGB) {
		double l = MonoGetLightness();
		m_srgb = { l, l, l };
	} else if (m_mathMode == MathMode::Linear_RGB) {
		double l = MonoGetLightness();
		m_lrgb = { l, l, l };
	} else {
		m_oklab.a = 0.;
		m_oklab.b = 0.;
	}
	Update();
	m_isGrayscale = true;
}

void Colour::Abs() {
	if (m_mathMode == MathMode::sRGB) {
		m_srgb.r = std::abs(m_srgb.r);
		m_srgb.g = std::abs(m_srgb.g);
		m_srgb.b = std::abs(m_srgb.b);

	} else if (m_mathMode == MathMode::Linear_RGB) {
		m_lrgb.r = std::abs(m_lrgb.r);
		m_lrgb.g = std::abs(m_lrgb.g);
		m_lrgb.b = std::abs(m_lrgb.b);
	} else {
		m_oklab.l = std::abs(m_oklab.l);
		m_oklab.a = std::abs(m_oklab.a);
		m_oklab.b = std::abs(m_oklab.b);
	}

	m_alpha = std::abs(m_alpha);
}

Colour Colour::Min(const Colour& a, const Colour& b) {
	Colour out;
	if (m_mathMode == MathMode::sRGB) {
		out.m_srgb.r = a.m_srgb.r < b.m_srgb.r ? a.m_srgb.r : b.m_srgb.r;
		out.m_srgb.g = a.m_srgb.g < b.m_srgb.g ? a.m_srgb.g : b.m_srgb.g;
		out.m_srgb.b = a.m_srgb.b < b.m_srgb.b ? a.m_srgb.b : b.m_srgb.b;
	} else if (m_mathMode == MathMode::Linear_RGB) {
		out.m_lrgb.r = a.m_lrgb.r < b.m_lrgb.r ? a.m_lrgb.r : b.m_lrgb.r;
		out.m_lrgb.g = a.m_lrgb.g < b.m_lrgb.g ? a.m_lrgb.g : b.m_lrgb.g;
		out.m_lrgb.b = a.m_lrgb.b < b.m_lrgb.b ? a.m_lrgb.b : b.m_lrgb.b;
	} else {
		out.m_oklab.l = a.m_oklab.l < b.m_oklab.l ? a.m_oklab.l : b.m_oklab.l;
		out.m_oklab.a = a.m_oklab.a < b.m_oklab.a ? a.m_oklab.a : b.m_oklab.a;
		out.m_oklab.b = a.m_oklab.b < b.m_oklab.b ? a.m_oklab.b : b.m_oklab.b;
	}
	out.m_alpha = a.m_alpha < b.m_alpha ? a.m_alpha : b.m_alpha;

	//out.Update();
	return out;
}

void Colour::OkLabFallback() {
	if (m_oklab.l == 1. || m_oklab.l == 0.) {
		m_oklab.a = 0.;
		m_oklab.b = 0.;
		return;
	}

	const int maxIter = 12;
	struct LCH { double l, c, h; };

	Colour s0 = Colour::FromOkLab(m_oklab.l, m_oklab.a, m_oklab.b, m_alpha);

	auto inGamut = [](const Colour& s) {
		return s.m_srgb.r >= 0. && s.m_srgb.r <= 1. &&
			s.m_srgb.g >= 0. && s.m_srgb.g <= 1. &&
			s.m_srgb.b >= 0. && s.m_srgb.b <= 1.;
		};

	if (inGamut(s0)) return;

	LCH lch = {
		m_oklab.l,
		std::sqrt(m_oklab.a * m_oklab.a + m_oklab.b * m_oklab.b),
		std::atan2(m_oklab.b, m_oklab.a)
	};

	auto LChToLab = [](const LCH& v) {
		OkLab out = { v.l, v.c * std::cos(v.h), v.c * std::sin(v.h) };
		return out;
		};

	double lo = 0., hi = lch.c;
	for (int i = 0; i < maxIter; ++i) {
		double mid = 0.5 * (lo + hi);
		OkLab test = LChToLab({ lch.l, mid, lch.h });

		Colour s = FromOkLab(test.l, test.a, test.b);

		if (inGamut(s)) {
			lo = mid;
		} else {
			hi = mid;
		}
	}

	m_oklab = LChToLab({ lch.l, lo, lch.h });
}

void Colour::sRGBtoLRGB() {
	constexpr double Y = 2.4125093745073549;
	constexpr double C = 0.056317370387926696;
	constexpr double A = 12.920750283132739;
	constexpr double X = 0.039870440086508217;
	if (m_srgb.r == m_srgb.g && m_srgb.r == m_srgb.b) {
		// grayscale - to avoid extra calculations
		m_isGrayscale = true;

		const double v = m_srgb.r <= X ? m_srgb.r / A : std::pow((m_srgb.r + C) / (C + 1.), Y);
		m_lrgb = { v, v, v };
	} else {
		m_isGrayscale = false;
		m_lrgb = {
			m_srgb.r <= X ? m_srgb.r / A : std::pow((m_srgb.r + C) / (C + 1.), Y),
			m_srgb.g <= X ? m_srgb.g / A : std::pow((m_srgb.g + C) / (C + 1.), Y),
			m_srgb.b <= X ? m_srgb.b / A : std::pow((m_srgb.b + C) / (C + 1.), Y)
		};
	}
}

void Colour::LRGBtosRGB() {
	constexpr double Y = 2.4125093745073549;
	constexpr double C = 0.056317370387926696;
	constexpr double A = 12.920750283132739;
	constexpr double X = 0.0030857681800844569;

	if (m_lrgb.r == m_lrgb.g && m_lrgb.r == m_lrgb.b) {
		// grayscale - to avoid extra calculations
		m_isGrayscale = true;

		const double v = m_lrgb.r <= X ? A * m_lrgb.r : (Maths::NRoot(m_lrgb.r, Y) * (C + 1.)) - C;
		m_srgb = { v, v, v };
	} else {
		m_isGrayscale = false;
		m_srgb = {
			m_lrgb.r <= X ? A * m_lrgb.r : (Maths::NRoot(m_lrgb.r, Y) * (C + 1.)) - C,
			m_lrgb.g <= X ? A * m_lrgb.g : (Maths::NRoot(m_lrgb.g, Y) * (C + 1.)) - C,
			m_lrgb.b <= X ? A * m_lrgb.b : (Maths::NRoot(m_lrgb.b, Y) * (C + 1.)) - C
		};
	}
}

void Colour::LRGBtoOkLab() {
	if (m_lrgb.r == m_lrgb.g && m_lrgb.r == m_lrgb.b) {
		// if graycale - can skip some conversions
		m_isGrayscale = true;

		double l = m_lrgb.r;
		// to LMS - can skip "to Linear LMS" conversion
		l = std::cbrt(l);

		// can skip "to OkLab" conversion
		m_oklab = { l, 0., 0. };
	} else {
		m_isGrayscale = false;

		double l1 = m_lrgb.r;
		double a1 = m_lrgb.g;
		double b1 = m_lrgb.b;

		// to Linear LMS

		double l2 = 0.4122214708 * l1 + 0.5363325363 * a1 + 0.0514459929 * b1;
		double a2 = 0.2119034982 * l1 + 0.6806995451 * a1 + 0.1073969566 * b1;
		double b2 = 0.0883024619 * l1 + 0.2817188376 * a1 + 0.6299787005 * b1;

		// to LMS
		l1 = std::cbrt(l2);
		a1 = std::cbrt(a2);
		b1 = std::cbrt(b2);

		// to OkLab
		l2 = 0.2104542553 * l1 + 0.7936177850 * a1 - 0.0040720468 * b1;
		a2 = 1.9779984951 * l1 - 2.4285922050 * a1 + 0.4505937099 * b1;
		b2 = 0.0259040371 * l1 + 0.7827717662 * a1 - 0.8086757660 * b1;

		m_oklab = { l2, a2, b2 };
	}
}

void Colour::OkLabtoLRGB() {
	if (m_oklab.a == 0. && m_oklab.b == 0) {
		// if graycale - can skip some conversions
		m_isGrayscale = true;

		double r = m_oklab.l;

		// to Linear LMS - can skip "to LMS" conversion
		r = r * r * r;

		m_lrgb = { r, r, r };
	} else {
		m_isGrayscale = false;
		double r1 = m_oklab.l;
		double g1 = m_oklab.a;
		double b1 = m_oklab.b;

		// to LMS

		double r2 = r1 + 0.3963377774 * g1 + 0.2158037573 * b1;
		double g2 = r1 - 0.1055613458 * g1 - 0.0638541728 * b1;
		double b2 = r1 - 0.0894841775 * g1 - 1.2914855480 * b1;

		// to Linear LMS
		r1 = r2 * r2 * r2;
		g1 = g2 * g2 * g2;
		b1 = b2 * b2 * b2;

		// to Linear RGB
		r2 = +4.0767416621 * r1 - 3.3077115913 * g1 + 0.2309699292 * b1;
		g2 = -1.2684380046 * r1 + 2.6097574011 * g1 - 0.3413193965 * b1;
		b2 = -0.0041960863 * r1 - 0.7034186147 * g1 + 1.7076147010 * b1;

		m_lrgb = { r2, g2, b2 };
	}
}

Colour::sRGB_UInt Colour::GetsRGB_UInt() const {
	double r = m_srgb.r * 255.;
	double g = m_srgb.g * 255.;
	double b = m_srgb.b * 255.;
	double a = m_alpha * 255.;

	r = std::round(r);
	g = std::round(g);
	b = std::round(b);
	a = std::round(a);

	r = r > 255. ? 255. : r;
	g = g > 255. ? 255. : g;
	b = b > 255. ? 255. : b;
	a = a > 255. ? 255. : a;

	r = r < 0. ? 0. : r;
	g = g < 0. ? 0. : g;
	b = b < 0. ? 0. : b;
	a = a < 0. ? 0. : a;

	return { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
}

void Colour::SetsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	m_srgb = { (double)r / 255., (double)g / 255., (double)b / 255. };
	m_alpha = (double)a / 255.;

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	sRGBtoLRGB();
	LRGBtoOkLab();
}

void Colour::SetsRGB_D(const double r, const double g, const double b, const double a) {
	m_srgb = { r, g, b };
	m_alpha = a;

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	sRGBtoLRGB();
	LRGBtoOkLab();
}

void Colour::SetOkLab(const double l, const double a, const double b, const double alpha) {
	m_alpha = alpha;
	m_oklab = { l, a, b };

	m_isGrayscale = false;
	if (a == 0. && b == 0.) m_isGrayscale = true;

	OkLabtoLRGB();
	LRGBtosRGB();
}

void Colour::SetHex(const char* hex) {
	const unsigned int hexInt = std::stoi(hex, 0, 16);

	const unsigned int rMask = 0xFF0000;
	const unsigned int gMask = 0x00FF00;
	const unsigned int bMask = 0x0000FF;

	const uint8_t r = uint8_t((hexInt & rMask) >> 16);
	const uint8_t g = uint8_t((hexInt & gMask) >> 8);
	const uint8_t b = uint8_t(hexInt & bMask);
	SetsRGB(r, g, b);
}

void Colour::SetLRGB(const double r, const double g, const double b, const double a) {
	m_alpha = a;
	m_lrgb = { r, g, b };

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	LRGBtosRGB();
	LRGBtoOkLab();
}