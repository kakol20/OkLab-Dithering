#include "Colour.h"

#include "../wrapper/Maths.hpp"


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
	// black or white
	if (m_srgb.r == 255 || m_srgb.g == 255 || m_srgb.b == 255) {
		m_oklab = { 1., 0., 0. };
		return;
	}
	if (m_srgb.r == 0 || m_srgb.g == 0 || m_srgb.b == 0) {
		m_oklab = { 0., 0., 0. };
		return;
	}

	const double scalar = 387916. / 30017.;
	const double limit = 11. / 280.;

	if (m_srgb.r == m_srgb.g && m_srgb.r == m_srgb.b) {
		// if graycale - can skip some conversions

		double l = double(m_srgb.r) / 255.;

		// to Linear RGB
		l = l <= limit ? l / scalar : std::pow((l + 0.055) / 1.055, 2.4);

		// to LMS - can skip "to Linear LMS" conversion
		l = std::cbrt(l);

		// can skip "to OkLab" conversion
		m_oklab = { l, 0., 0. };
	} else {
		double l1 = double(m_srgb.r) / 255.;
		double a1 = double(m_srgb.g) / 255.;
		double b1 = double(m_srgb.b) / 255.;

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

Colour Colour::FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Colour out;

	out.m_srgb = { r, g, b };
	out.m_alpha = (double)a / 255.;

	out.UpdateOkLab();
	return out;
}

//Colour::OkLab Colour::sRGBtoOkLab(const sRGB val) {
//
//	return OkLab();
//}