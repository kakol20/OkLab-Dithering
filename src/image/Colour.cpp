#include "Colour.h"

#include "../wrapper/Log.h"
#include "../wrapper/Maths.hpp"

Colour::MathMode Colour::m_mathMode = Colour::MathMode::OkLab_Lightness;

Colour::Colour() {
	m_srgb = { 0, 0, 0 };
	m_oklab = { 0., 0., 0. };
	m_alpha = 1.;
}

Colour::Colour(const Colour& other) {
	m_srgb = other.m_srgb;
	m_oklab = other.m_oklab;
	m_alpha = other.m_alpha;
}

Colour::~Colour() {
}

Colour& Colour::operator=(const Colour& other) {
	if (this == &other) return *this;
	m_srgb = other.m_srgb;
	m_oklab = other.m_oklab;
	m_alpha = other.m_alpha;
	return *this;
}

void Colour::UpdateOkLab() {
	const double scalar = 387916. / 30017.;
	const double limit = 11. / 280.;

	if (m_srgb.r == m_srgb.g && m_srgb.r == m_srgb.b) {
		// if graycale - can skip some conversions

		double l = m_srgb.r;

		// to Linear RGB
		l = l <= limit ? l / scalar : std::pow((l + 0.055) / 1.055, 2.4);

		// to LMS - can skip "to Linear LMS" conversion
		l = std::cbrt(l);

		// can skip "to OkLab" conversion
		m_oklab = { l, 0., 0. };
	} else {
		double l1 = m_srgb.r;
		double a1 = m_srgb.g;
		double b1 = m_srgb.b;

		// to Linear RGB
		l1 = l1 <= limit ? l1 / scalar : std::pow((l1 + 0.055) / 1.055, 2.4);
		a1 = a1 <= limit ? a1 / scalar : std::pow((a1 + 0.055) / 1.055, 2.4);
		b1 = b1 <= limit ? b1 / scalar : std::pow((b1 + 0.055) / 1.055, 2.4);

		// to Linear LMS

		double l2 = 0.412221470800 * l1 + 0.536332536300 * a1 + 0.051445992900 * b1;
		double a2 = 0.211903498234 * l1 + 0.680699545133 * a1 + 0.107396956633 * b1;
		double b2 = 0.088302461900 * l1 + 0.281718837600 * a1 + 0.629978700500 * b1;

		// to LMS
		l1 = std::cbrt(l2);
		a1 = std::cbrt(a2);
		b1 = std::cbrt(b2);

		// to OkLab
		l2 = 0.210454257467 * l1 + 0.793617787167 * a1 - 0.004072044634 * b1;
		a2 = 1.977998495100 * l1 - 2.428592205000 * a1 + 0.450593709900 * b1;
		b2 = 0.025904024666 * l1 + 0.782771753767 * a1 - 0.808675778433 * b1;

		m_oklab = { l2, a2, b2 };
	}
}

void Colour::UpdatesRGB() {
	const double scalar = 387916. / 30017.;
	const double limit = 285. / 93752.;

	if (m_oklab.a == 0 && m_oklab.b == 0) {
		// if graycale - can skip some conversions

		double r = m_oklab.l;

		// to Linear LMS - can skip "to LMS" conversion

		r = r * r * r;

		// to sRGB - can skip "to Linear RGB" conversion
		r = r <= limit ? scalar * r : (Maths::NRoot(r, 2.4) * 1.055) - 0.055;

		m_srgb = { r, r, r };
	} else {
		double r1 = m_oklab.l;
		double g1 = m_oklab.a;
		double b1 = m_oklab.b;

		// to LMS

		double r2 = r1 + 0.396337792278 * g1 + 0.215803757471 * b1;
		double g2 = r1 - 0.105561342920 * g1 - 0.063854171399 * b1;
		double b2 = r1 - 0.089484185764 * g1 - 1.291485517099 * b1;

		// to Linear LMS
		r1 = r2 * r2 * r2;
		g1 = g2 * g2 * g2;
		b1 = b2 * b2 * b2;

		// to Linear RGB
		r2 = 4.076741661667 * r1 - 3.307711590572 * g1 + 0.230969928905 * b1;
		g2 = -1.268438004344 * r1 + 2.609757400792 * g1 - 0.341319396448 * b1;
		b2 = -0.004196086474 * r1 - 0.703418614494 * g1 + 1.707614700968 * b1;

		// to sRGB
		r2 = r2 <= limit ? scalar * r2 : (Maths::NRoot(r2, 2.4) * 1.055) - 0.055;
		g2 = g2 <= limit ? scalar * g2 : (Maths::NRoot(g2, 2.4) * 1.055) - 0.055;
		b2 = b2 <= limit ? scalar * b2 : (Maths::NRoot(b2, 2.4) * 1.055) - 0.055;

		m_srgb = { r2, g2, b2 };
	}
}

void Colour::Update() {
	if (m_mathMode == MathMode::sRGB) {
		UpdateOkLab();
	} else {
		UpdatesRGB();
	}
}

Colour Colour::FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Colour out;

	out.m_srgb = { (double)r / 255., (double)g / 255., (double)b / 255. };
	out.m_alpha = (double)a / 255.;

	out.UpdateOkLab();
	return out;
}

Colour Colour::FromsRGB_D(const double r, const double g, const double b, const double a) {
	Colour out;
	out.m_srgb = { r, g, b };
	out.m_alpha = a;
	out.UpdateOkLab();
	return out;
}

Colour Colour::FromOkLab(const double l, const double a, const double b, const double alpha) {
	Colour out;

	out.m_oklab = { l, a, b };
	out.m_alpha = alpha;

	out.UpdatesRGB();
	return out;
}

Colour Colour::FromHex(const char* hex) {
	const unsigned int hexInt = std::stoi(hex, 0, 16);

	const unsigned int rMask = 0xFF0000;
	const unsigned int gMask = 0x00FF00;
	const unsigned int bMask = 0x0000FF;

	const uint8_t r = uint8_t((hexInt & rMask) >> 16);
	const uint8_t g = uint8_t((hexInt & gMask) >> 8);
	const uint8_t b = uint8_t(hexInt & bMask);
	return FromsRGB(r, g, b, 255);
}

void Colour::Clamp() {
	if (m_mathMode == MathMode::sRGB) {
		m_srgb.r = m_srgb.r > 1. ? 1. : m_srgb.r;
		m_srgb.r = m_srgb.r < 0. ? 0. : m_srgb.r;

		m_srgb.g = m_srgb.g > 1. ? 1. : m_srgb.g;
		m_srgb.g = m_srgb.g < 0. ? 0. : m_srgb.g;

		m_srgb.b = m_srgb.b > 1. ? 1. : m_srgb.b;
		m_srgb.b = m_srgb.b < 0. ? 0. : m_srgb.b;
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
	}
	return 0.0;
}

void Colour::OkLabFallback() {
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

Colour::sRGB_UInt Colour::GetsRGB_UInt() const {
	double r = m_srgb.r * 255.;
	double g = m_srgb.g * 255.;
	double b = m_srgb.b * 255.;

	r = std::round(r);
	g = std::round(g);
	b = std::round(b);

	r = r > 255. ? 255. : r;
	g = g > 255. ? 255. : g;
	b = b > 255. ? 255. : b;

	r = r < 0. ? 0. : r;
	g = g < 0. ? 0. : g;
	b = b < 0. ? 0. : b;

	return { (uint8_t)r, (uint8_t)g, (uint8_t)b };
}

void Colour::SetsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	m_srgb = { (double)r / 255., (double)g / 255., (double)b / 255. };
	m_alpha = (double)a / 255.;

	UpdateOkLab();
}

void Colour::SetOkLab(const double l, const double a, const double b, const double alpha) {
	m_alpha = alpha;
	m_oklab = { l, a, b };

	UpdatesRGB();
}