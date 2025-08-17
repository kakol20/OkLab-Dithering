#include "Colour.h"

Colour::Colour() {
	m_srgb.r = 0;
	m_srgb.g = 0;
	m_srgb.b = 0;
	m_alpha = 1.;
}

Colour::Colour(const Colour& other) {
	m_srgb = other.m_srgb;
}

Colour::~Colour() {
}

Colour& Colour::operator=(const Colour& other) {
	if (this == &other) return *this;
	m_srgb = other.m_srgb;
	return *this;
}

Colour Colour::FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Colour out;

	out.m_srgb.r = r;
	out.m_srgb.g = g;
	out.m_srgb.b = b;
	out.m_alpha = (double)a / 255.;
	return out;
}
