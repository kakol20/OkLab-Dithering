#define _USE_MATH_DEFINES

#include "../wrapper/Log.h"
#include "../wrapper/Maths.hpp"
#include "Colour.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

constexpr double M_TAU = M_PI * 2;

Colour::MathMode Colour::m_mathMode = Colour::MathMode::OkLab_Lightness;

Colour::Colour() {
	m_alpha = 1.;
	m_isGrayscale = false;
	m_lrgb = { 0., 0., 0. };
	m_oklab = { 0., 0., 0. };
	m_srgb = { 0, 0, 0 };
	m_oklch = { 0., 0., 0. };
	m_srgbUint = { 0, 0, 0 };
}

Colour::Colour(const Colour& other) {
	m_alpha = other.m_alpha;
	m_isGrayscale = other.m_isGrayscale;
	m_lrgb = other.m_lrgb;
	m_oklab = other.m_oklab;
	m_srgb = other.m_srgb;
	m_oklch = other.m_oklch;
	m_srgbUint = other.m_srgbUint;
}

Colour::Colour(const double l, const double a, const double b, const double alpha) {
	SetOkLab(l, a, b, alpha);
}

Colour::Colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	m_srgbUint = { r, g, b };
	SetsRGB(r, g, b, a);
}

Colour::Colour(const char* hex) {
	SetHex(hex);
}

Colour::~Colour() {}

Colour& Colour::operator=(const Colour& other) {
	if (this == &other) return *this;
	m_alpha = other.m_alpha;
	m_isGrayscale = other.m_isGrayscale;
	m_lrgb = other.m_lrgb;
	m_oklab = other.m_oklab;
	m_srgb = other.m_srgb;
	m_oklch = other.m_oklch;
	m_srgbUint = other.m_srgbUint;
	return *this;
}

void Colour::Update() {
	if (m_mathMode == MathMode::sRGB) {
		sRGBtoLRGB();
		LRGBtoOkLab();
		OkLabToOkLCh();
		sRGBToUint();
	} else if (m_mathMode == MathMode::Linear_RGB) {
		LRGBtosRGB();
		LRGBtoOkLab();
		OkLabToOkLCh();
		sRGBToUint();
	} else if (m_mathMode == MathMode::OkLCh) {
		OkLChToOkLAB();
		OkLabtoLRGB();
		LRGBtosRGB();
		sRGBToUint();
	} else if (m_mathMode == MathMode::sRGB_Uint) {
		UintTosRGB();
		sRGBtoLRGB();
		LRGBtoOkLab();
		OkLabToOkLCh();
	} else {
		OkLabtoLRGB();
		LRGBtosRGB();
		OkLabToOkLCh();
		sRGBToUint();
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

Colour Colour::FromLCH(const double l, const double c, const double h, const double alpha) {
	Colour out;
	out.SetOkLCh(l, c, h, alpha);
	return out;
}

void Colour::Clamp() {
	switch (m_mathMode) {
	case Colour::MathMode::sRGB:
		m_srgb.r = m_srgb.r > 1. ? 1. : m_srgb.r;
		m_srgb.r = m_srgb.r < 0. ? 0. : m_srgb.r;

		m_srgb.g = m_srgb.g > 1. ? 1. : m_srgb.g;
		m_srgb.g = m_srgb.g < 0. ? 0. : m_srgb.g;

		m_srgb.b = m_srgb.b > 1. ? 1. : m_srgb.b;
		m_srgb.b = m_srgb.b < 0. ? 0. : m_srgb.b;
		break;
	case Colour::MathMode::OkLab:
		m_oklab.l = m_oklab.l > 1. ? 1. : m_oklab.l;
		m_oklab.l = m_oklab.l < 0. ? 0. : m_oklab.l;

		OkLabFallback();
		break;
	case Colour::MathMode::OkLab_Lightness:
		m_oklab.l = m_oklab.l > 1. ? 1. : m_oklab.l;
		m_oklab.l = m_oklab.l < 0. ? 0. : m_oklab.l;

		OkLabFallback();
		break;
	case Colour::MathMode::Linear_RGB:
		m_lrgb.r = m_lrgb.r > 1. ? 1. : m_lrgb.r;
		m_lrgb.r = m_lrgb.r < 0. ? 0. : m_lrgb.r;

		m_lrgb.g = m_lrgb.g > 1. ? 1. : m_lrgb.g;
		m_lrgb.g = m_lrgb.g < 0. ? 0. : m_lrgb.g;

		m_lrgb.b = m_lrgb.b > 1. ? 1. : m_lrgb.b;
		m_lrgb.b = m_lrgb.b < 0. ? 0. : m_lrgb.b;
		break;
	case Colour::MathMode::OkLCh:
		m_oklch.l = m_oklch.l > 1. ? 1. : m_oklch.l;
		m_oklch.l = m_oklch.l < 0. ? 0. : m_oklch.l;

		m_oklch.h = m_oklch.h > M_PI * 2. ? M_PI * 2. : m_oklch.h;
		m_oklch.h = m_oklch.h < 0. ? 0. : m_oklch.h;
		OkLChFallback();
		break;
	case Colour::MathMode::sRGB_Uint:
		m_srgbUint.r = m_srgbUint.r > 255 ? 255 : m_srgbUint.r;
		m_srgbUint.r = m_srgbUint.r < 0 ? 0 : m_srgbUint.r;

		m_srgbUint.g = m_srgbUint.g > 255 ? 255 : m_srgbUint.g;
		m_srgbUint.g = m_srgbUint.g < 0 ? 0 : m_srgbUint.g;

		m_srgbUint.b = m_srgbUint.b > 255 ? 255 : m_srgbUint.b;
		m_srgbUint.b = m_srgbUint.b < 0 ? 0 : m_srgbUint.b;
		break;
	default:
		break;
	}

	m_alpha = m_alpha > 1. ? 1. : m_alpha;
	m_alpha = m_alpha < 0. ? 0. : m_alpha;
}

Colour& Colour::operator/=(const Colour& other) {
	double r = 0, g = 0, b = 0;
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
	case MathMode::OkLCh:
		OkLChToOkLAB();
		m_oklab.l /= other.m_oklab.l;
		m_oklab.a /= other.m_oklab.a;
		m_oklab.b /= other.m_oklab.b;
		OkLabToOkLCh();
		break;
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r) / other.m_srgbUint.r;
		g = static_cast<double>(m_srgbUint.g) / other.m_srgbUint.g;
		b = static_cast<double>(m_srgbUint.b) / other.m_srgbUint.b;

		r = std::floor(r);
		g = std::floor(g);
		b = std::floor(b);

		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		g = g > 255. ? 255. : g;
		g = g < 0. ? 0. : g;

		b = b > 255. ? 255. : b;
		b = b < 0. ? 0. : b;

		m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
		break;
	default:
		break;
	}
	return *this;
}

Colour& Colour::operator*=(const Colour& other) {
	double r = 0, g = 0, b = 0;
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
	case MathMode::OkLCh:
		OkLChToOkLAB();
		m_oklab.l *= other.m_oklab.l;
		m_oklab.a *= other.m_oklab.a;
		m_oklab.b *= other.m_oklab.b;
		OkLabToOkLCh();
		break;
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r) * other.m_srgbUint.r;
		g = static_cast<double>(m_srgbUint.g) * other.m_srgbUint.g;
		b = static_cast<double>(m_srgbUint.b) * other.m_srgbUint.b;

		r = std::floor(r);
		g = std::floor(g);
		b = std::floor(b);

		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		g = g > 255. ? 255. : g;
		g = g < 0. ? 0. : g;

		b = b > 255. ? 255. : b;
		b = b < 0. ? 0. : b;

		m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
		break;
	default:
		break;
	}
	return *this;
}

Colour& Colour::operator+=(const Colour& other) {
	double r = 0, g = 0, b = 0;
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
	case MathMode::OkLCh:
		OkLChToOkLAB();
		m_oklab.l += other.m_oklab.l;
		m_oklab.a += other.m_oklab.a;
		m_oklab.b += other.m_oklab.b;
		OkLabToOkLCh();
		break;
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r) + other.m_srgbUint.r;
		g = static_cast<double>(m_srgbUint.g) + other.m_srgbUint.g;
		b = static_cast<double>(m_srgbUint.b) + other.m_srgbUint.b;

		r = std::floor(r);
		g = std::floor(g);
		b = std::floor(b);

		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		g = g > 255. ? 255. : g;
		g = g < 0. ? 0. : g;

		b = b > 255. ? 255. : b;
		b = b < 0. ? 0. : b;

		m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
		break;
	default:
		break;
	}
	return *this;
}

Colour& Colour::operator-=(const Colour& other) {
	double r = 0, g = 0, b = 0;
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
	case MathMode::OkLCh:
		OkLChToOkLAB();
		m_oklab.l -= other.m_oklab.l;
		m_oklab.a -= other.m_oklab.a;
		m_oklab.b -= other.m_oklab.b;
		OkLabToOkLCh();
		break;
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r) - other.m_srgbUint.r;
		g = static_cast<double>(m_srgbUint.g) - other.m_srgbUint.g;
		b = static_cast<double>(m_srgbUint.b) - other.m_srgbUint.b;

		r = std::floor(r);
		g = std::floor(g);
		b = std::floor(b);

		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		g = g > 255. ? 255. : g;
		g = g < 0. ? 0. : g;

		b = b > 255. ? 255. : b;
		b = b < 0. ? 0. : b;

		m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
		break;
	default:
		break;
	}
	return *this;
}

Colour& Colour::operator*=(const double scalar) {
	double r = 0, g = 0, b = 0;
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
	case MathMode::OkLCh:
		OkLChToOkLAB();
		m_oklab.l *= scalar;
		m_oklab.a *= scalar;
		m_oklab.b *= scalar;
		OkLabToOkLCh();
		break;
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r) * scalar;
		g = static_cast<double>(m_srgbUint.g) * scalar;
		b = static_cast<double>(m_srgbUint.b) * scalar;

		r = std::floor(r);
		g = std::floor(g);
		b = std::floor(b);

		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		g = g > 255. ? 255. : g;
		g = g < 0. ? 0. : g;

		b = b > 255. ? 255. : b;
		b = b < 0. ? 0. : b;

		m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
		break;
	default:
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

bool Colour::operator==(const Colour& other) const {
	switch (m_mathMode) {
	case Colour::MathMode::sRGB:
		return std::tie(m_srgb.r, m_srgb.g, m_srgb.b, m_alpha) ==
			std::tie(other.m_srgb.r, other.m_srgb.g, other.m_srgb.b, other.m_alpha);
	case Colour::MathMode::OkLab:
		return std::tie(m_oklab.l, m_oklab.a, m_oklab.b, m_alpha) ==
			std::tie(other.m_oklab.l, other.m_oklab.a, other.m_oklab.b, other.m_alpha);
	case Colour::MathMode::OkLab_Lightness:
		return std::tie(m_oklab.l, m_alpha) ==
			std::tie(other.m_oklab.l, other.m_alpha);
	case Colour::MathMode::Linear_RGB:
		return std::tie(m_lrgb.r, m_lrgb.g, m_lrgb.b, m_alpha) ==
			std::tie(other.m_lrgb.r, other.m_lrgb.g, other.m_lrgb.b, other.m_alpha);
	case Colour::MathMode::OkLCh:
		return std::tie(m_oklch.l, m_oklch.c, m_oklch.h, m_alpha) ==
			std::tie(other.m_oklch.l, m_oklch.c, m_oklch.h, other.m_alpha);
	case Colour::MathMode::sRGB_Uint:
		return std::tie(m_srgbUint.r, m_srgbUint.g, m_srgbUint.b, m_alpha) ==
			std::tie(other.m_srgbUint.r, other.m_srgbUint.g, other.m_srgbUint.b, other.m_alpha);
	default:
		return false;
	}
}

bool Colour::operator<(const Colour& other) const {
	if (m_mathMode == Colour::MathMode::sRGB) {
		return std::tie(m_srgb.r, m_srgb.g, m_srgb.b, m_alpha) <
			std::tie(other.m_srgb.r, other.m_srgb.g, other.m_srgb.b, other.m_alpha);
	} else if (m_mathMode == Colour::MathMode::OkLab) {
		return std::tie(m_oklab.l, m_oklab.a, m_oklab.b, m_alpha) <
			std::tie(other.m_oklab.l, other.m_oklab.a, other.m_oklab.b, other.m_alpha);
	} else if (m_mathMode == Colour::MathMode::OkLab_Lightness) {
		return std::tie(m_oklab.l, m_alpha) <
			std::tie(other.m_oklab.l, other.m_alpha);
	} else if (m_mathMode == Colour::MathMode::Linear_RGB) {
		return std::tie(m_lrgb.r, m_lrgb.g, m_lrgb.b, m_alpha) <
			std::tie(other.m_lrgb.r, other.m_lrgb.g, other.m_lrgb.b, other.m_alpha);
	} else if (m_mathMode == Colour::MathMode::sRGB_Uint) {
		return std::tie(m_srgbUint.r, m_srgbUint.g, m_srgbUint.b, m_alpha) <
			std::tie(other.m_srgbUint.r, other.m_srgbUint.g, other.m_srgbUint.b, other.m_alpha);
	} else {
		const Colour red(static_cast<uint8_t>(255), 0, 0);
		const double n = 12.;
		const double offset = -red.m_oklch.h + (M_PI / n);

		double currH = m_oklch.h + offset;
		double otherH = other.m_oklch.h + offset;

		// Wrapped clamp
		currH = currH < 0. ? currH + M_TAU : currH;
		otherH = otherH < 0. ? otherH + M_TAU : otherH;

		currH = currH >= M_TAU ? currH - M_TAU : currH;
		otherH = otherH >= M_TAU ? otherH - M_TAU : otherH;

		// Rounding
		currH = (std::floor((n * currH) / M_TAU) * M_TAU) / (n - 1.);
		otherH = (std::floor((n * otherH) / M_TAU) * M_TAU) / (n - 1.);

		// check grayscale
		if (m_oklch.c == 0) currH = -1.;
		if (other.m_oklch.c == 0) otherH = -1.;

		if (currH != otherH) return currH < otherH;

		if (m_oklch.l != other.m_oklch.l) return m_oklch.l < other.m_oklch.l;
		if (m_oklch.c != other.m_oklch.c) return m_oklch.c < other.m_oklch.c;
		return m_alpha < other.m_alpha;
	}

	return false;
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
	return Log::ToString((unsigned int)m_srgbUint.r, 3, ' ') + ' ' +
		Log::ToString((unsigned int)m_srgbUint.g, 3, ' ') + ' ' +
		Log::ToString((unsigned int)m_srgbUint.b, 3, ' ');
}

std::string Colour::OkLChDebug() const {
	std::string lStr = Log::LeadingCharacter(Log::ToString(m_oklch.l, 4), 7, ' ');
	std::string cStr = Log::LeadingCharacter(Log::ToString(m_oklch.c, 4), 7, ' ');
	std::string hStr = Log::LeadingCharacter(Log::ToString(m_oklch.h * (180. / M_PI), 2), 7, ' ');
	return lStr + ' ' + cStr + ' ' + hStr;
}

std::string Colour::GetHex() const {
	const unsigned int r = static_cast<unsigned int>(m_srgbUint.r) << 16;
	const unsigned int g = static_cast<unsigned int>(m_srgbUint.g) << 8;
	const unsigned int b = static_cast<unsigned int>(m_srgbUint.b);
	const unsigned int col = r + g + b;

	std::stringstream stream;
	stream << std::setfill('0') << std::setw(6) << std::uppercase << std::hex << col;
	return stream.str();
}

double Colour::MagSq(const Colour& other) const {
	double r = 0., g = 0., b = 0.;
	double otherR = 0., otherG = 0., otherB = 0.;

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
		return (m_oklab.l - other.m_oklab.l) * (m_oklab.l - other.m_oklab.l);
	case MathMode::Linear_RGB:
		return Maths::Pow2(m_lrgb.r - other.m_lrgb.r) +
			Maths::Pow2(m_lrgb.g - other.m_lrgb.g) +
			Maths::Pow2(m_lrgb.b - other.m_lrgb.b);
	case MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r);
		g = static_cast<double>(m_srgbUint.g);
		b = static_cast<double>(m_srgbUint.b);

		otherR = static_cast<double>(other.m_srgbUint.r);
		otherG = static_cast<double>(other.m_srgbUint.g);
		otherB = static_cast<double>(other.m_srgbUint.b);

		return Maths::Pow2(r - otherR) +
			Maths::Pow2(g - otherG) +
			Maths::Pow2(b - otherB);
	}
	return 0.0;
}

double Colour::LengthSq() const {
	double r = 0., g = 0., b = 0.;

	switch (m_mathMode) {
	case Colour::MathMode::sRGB:
		return Maths::Pow2(m_srgb.r) +
			Maths::Pow2(m_srgb.g) +
			Maths::Pow2(m_srgb.b);
	case Colour::MathMode::OkLab:
		return Maths::Pow2(m_oklab.l) +
			Maths::Pow2(m_oklab.a) +
			Maths::Pow2(m_oklab.b);
	case Colour::MathMode::OkLab_Lightness:
		return Maths::Pow2(m_oklab.l);
	case Colour::MathMode::Linear_RGB:
		return Maths::Pow2(m_lrgb.r) +
			Maths::Pow2(m_lrgb.g) +
			Maths::Pow2(m_lrgb.b);
	case Colour::MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r);
		g = static_cast<double>(m_srgbUint.g);
		b = static_cast<double>(m_srgbUint.b);

		return Maths::Pow2(r) +
			Maths::Pow2(g) +
			Maths::Pow2(b);
	default:
		return 0.;
	}
}

double Colour::Dot(const Colour& other) const {
	double r = 0., g = 0., b = 0.;
	double otherR = 0., otherG = 0., otherB = 0.;

	switch (m_mathMode) {
	case Colour::MathMode::sRGB:
		return m_srgb.r * other.m_srgb.r +
			m_srgb.g * other.m_srgb.g +
			m_srgb.b * other.m_srgb.b;
	case Colour::MathMode::OkLab:
		return m_oklab.l * other.m_oklab.l +
			m_oklab.a * other.m_oklab.a +
			m_oklab.b * other.m_oklab.b;
	case Colour::MathMode::OkLab_Lightness:
		return m_oklab.l * other.m_oklab.l;
	case Colour::MathMode::Linear_RGB:
		return m_lrgb.r * other.m_lrgb.r +
			m_lrgb.g * other.m_lrgb.g +
			m_lrgb.b * other.m_lrgb.b;
	case Colour::MathMode::sRGB_Uint:
		r = static_cast<double>(m_srgbUint.r);
		g = static_cast<double>(m_srgbUint.g);
		b = static_cast<double>(m_srgbUint.b);

		otherR = static_cast<double>(other.m_srgbUint.r);
		otherG = static_cast<double>(other.m_srgbUint.g);
		otherB = static_cast<double>(other.m_srgbUint.b);
		return r * otherR +
			g * otherG +
			b * otherB;
	default:
		return 0.0;
	}
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
	} else if (m_mathMode == MathMode::OkLCh) {
		return m_oklch.l;
	} else if (m_mathMode == MathMode::sRGB_Uint) {
		double r = static_cast<double>(m_srgbUint.r);
		double g = static_cast<double>(m_srgbUint.g);
		double b = static_cast<double>(m_srgbUint.b);
		return 0.2126 * r + 0.7152 * g + 0.0722 * b;
	} else {
		return m_oklab.l;
	}
}

void Colour::ToGrayscale() {
	double l = MonoGetLightness();
	if (m_mathMode == MathMode::sRGB) {
		m_srgb = { l, l, l };
	} else if (m_mathMode == MathMode::Linear_RGB) {
		m_lrgb = { l, l, l };
	} else if (m_mathMode == MathMode::OkLCh) {
		m_oklch.c = 0.;
		m_oklch.h = 0.;
	} else if (m_mathMode == MathMode::sRGB_Uint) {
		l = std::floor(l);
		l = l > 255. ? 255 : l < 0. ? 0. : l;
		const uint8_t l_uint8 = static_cast<uint8_t>(l);
		m_srgbUint = { l_uint8, l_uint8, l_uint8 };
	} else {
		m_oklab.a = 0.;
		m_oklab.b = 0.;
	}
	Update();
	m_isGrayscale = true;
}

void Colour::Abs() {
	switch (m_mathMode) {
	case Colour::MathMode::sRGB:
		m_srgb.r = std::abs(m_srgb.r);
		m_srgb.g = std::abs(m_srgb.g);
		m_srgb.b = std::abs(m_srgb.b);
		break;
	case Colour::MathMode::OkLab:
		m_oklab.l = std::abs(m_oklab.l);
		m_oklab.a = std::abs(m_oklab.a);
		m_oklab.b = std::abs(m_oklab.b);
		break;
	case Colour::MathMode::OkLab_Lightness:
		m_oklab.l = std::abs(m_oklab.l);
		break;
	case Colour::MathMode::Linear_RGB:
		m_lrgb.r = std::abs(m_lrgb.r);
		m_lrgb.g = std::abs(m_lrgb.g);
		m_lrgb.b = std::abs(m_lrgb.b);
		break;
	default:
		break;
	}

	m_alpha = std::abs(m_alpha);
}

void Colour::PureBlack(const uint8_t alpha) {
	/*
		m_alpha = other.m_alpha;
	m_isGrayscale = other.m_isGrayscale;
	m_lrgb = other.m_lrgb;
	m_oklab = other.m_oklab;
	m_srgb = other.m_srgb;
	*/
	m_srgb = { 0., 0., 0. };
	m_lrgb = { 0., 0., 0. };
	m_oklab = { 0., 0., 0. };
	m_oklch = { 0., 0., 0. };
	m_srgbUint = { 0, 0, 0 };
	m_isGrayscale = true;

	m_alpha = static_cast<double>(alpha) / 255.;
}

void Colour::OkLabFallback() {
	if (m_oklab.l >= 1. || m_oklab.l <= 0.) {
		m_oklab.l = std::clamp(m_oklab.l, 0., 1.);
		m_oklab.a = 0.;
		m_oklab.b = 0.;
		return;
	}

	Colour s0 = Colour::FromOkLab(m_oklab.l, m_oklab.a, m_oklab.b, m_alpha);
	s0.OkLChFallback();

	m_oklab = s0.m_oklab;
	OkLabToOkLCh();
	OkLabtoLRGB();
	LRGBtosRGB();
	sRGBToUint();
}

void Colour::OkLChFallback() {
	if (m_oklch.l >= 1. || m_oklch.l <= 0.) {
		m_oklch.l = std::clamp(m_oklch.l, 0., 1.);
		m_oklch.c = 0.;
		m_oklch.h = 0.;
		return;
	}

	const int maxIter = 12;

	Colour s0 = Colour::FromLCH(m_oklch.l, m_oklch.c, m_oklch.h, m_alpha);

	auto inGamut = [](const Colour& s) {
		return s.m_srgb.r >= 0. && s.m_srgb.r <= 1. &&
			s.m_srgb.g >= 0. && s.m_srgb.g <= 1. &&
			s.m_srgb.b >= 0. && s.m_srgb.b <= 1.;
		};

	if (inGamut(s0)) return;

	double lo = 0., hi = s0.m_oklch.c;
	for (int i = 0; i < maxIter; ++i) {
		double mid = 0.5 * (lo + hi);
		Colour s = FromLCH(s0.m_oklch.l, mid, s0.m_oklch.h);

		if (inGamut(s)) {
			lo = mid;
		} else {
			hi = mid;
		}
	}

	m_oklch = { s0.m_oklch.l, lo, s0.m_oklch.h };
	OkLChToOkLAB();
	OkLabtoLRGB();
	LRGBtosRGB();
	sRGBToUint();
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

void Colour::OkLabToOkLCh() {
	m_oklch = {
		m_oklab.l,
		std::sqrt(m_oklab.a * m_oklab.a + m_oklab.b * m_oklab.b),
		std::atan2(m_oklab.b, m_oklab.a)
	};
	if (m_oklch.h < 0.) m_oklch.h += M_TAU;
}

void Colour::OkLChToOkLAB() {
	m_oklab = {
		m_oklch.l,
		m_oklch.c * std::cos(m_oklch.h),
		m_oklch.c * std::sin(m_oklch.h)
	};
}

void Colour::sRGBToUint() {
	double r = m_srgb.r * 256.;
	double g = m_srgb.g * 256.;
	double b = m_srgb.b * 256.;

	r = std::floor(r);
	g = std::floor(g);
	b = std::floor(b);

	r = r > 255. ? 255. : r;
	g = g > 255. ? 255. : g;
	b = b > 255. ? 255. : b;

	r = r < 0. ? 0. : r;
	g = g < 0. ? 0. : g;
	b = b < 0. ? 0. : b;

	//return { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
	m_srgbUint = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
}

void Colour::UintTosRGB() {
	m_srgb = {
		static_cast<double>(m_srgbUint.r) / 255.,
		static_cast<double>(m_srgbUint.g) / 255.,
		static_cast<double>(m_srgbUint.b) / 255.
	};
}

void Colour::SetsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	m_srgbUint = { r, g, b };
	m_alpha = (double)a / 255.;

	if (m_alpha <= 0.) {
		m_srgb = { 0., 0., 0. };
	} else {
		m_srgb = { (double)r / 255., (double)g / 255., (double)b / 255. };
	}

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	sRGBtoLRGB();
	LRGBtoOkLab();
	OkLabToOkLCh();
}

void Colour::SetsRGB_D(const double r, const double g, const double b, const double a) {
	m_alpha = a;

	if (m_alpha <= 0.) {
		m_srgb = { 0., 0., 0. };
	} else {
		m_srgb = { r, g, b };
	}

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	sRGBtoLRGB();
	LRGBtoOkLab();
	OkLabToOkLCh();
	sRGBToUint();
}

void Colour::SetOkLCh(const double l, const double c, const double h, const double a) {
	m_alpha = a;
	if (m_alpha <= 0.) {
		m_oklch = { 0., 0., 0. };
	} else {
		m_oklch = { l, c, h };
	}

	m_isGrayscale = false;
	if (c == 0.) m_isGrayscale = true;

	OkLChToOkLAB();
	OkLabtoLRGB();
	LRGBtosRGB();
	sRGBToUint();
}

void Colour::SetOkLab(const double l, const double a, const double b, const double alpha) {
	m_alpha = alpha;

	if (m_alpha <= 0.) {
		m_oklab = { 0., 0., 0. };
	} else {
		m_oklab = { l, a, b };
	}

	m_isGrayscale = false;
	if (a == 0. && b == 0.) m_isGrayscale = true;

	OkLabtoLRGB();
	LRGBtosRGB();
	OkLabToOkLCh();
	sRGBToUint();
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

	if (m_alpha <= 0) {
		m_lrgb = { 0., 0., 0. };
	} else {
		m_lrgb = { r, g, b };
	}

	m_isGrayscale = false;
	if (r == g && r == b) m_isGrayscale = true;

	LRGBtosRGB();
	LRGBtoOkLab();
	OkLabToOkLCh();
	sRGBToUint();
}