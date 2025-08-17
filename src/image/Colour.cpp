#include "Colour.h"

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
		r *= 255.;

		// clamp value
		r = r > 255. ? 255. : r;
		r = r < 0. ? 0. : r;

		const double r_int = (double)r;

		m_srgb = { r_int, r_int, r_int };
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

		r2 *= 255.;
		g2 *= 255.;
		b2 *= 255.;

		// clamp values
		r2 = r2 >= 255. ? 255. : r2;
		g2 = g2 >= 255. ? 255. : g2;
		b2 = b2 >= 255. ? 255. : b2;

		r2 = r2 <= 0. ? 0. : r2;
		g2 = g2 <= 0. ? 0. : g2;
		b2 = b2 <= 0. ? 0. : b2;

		const double r_int = double(std::round(r2));
		const double g_int = double(std::round(g2));
		const double b_int = double(std::round(b2));

		m_srgb = { r_int, g_int, b_int };
	}
}

Colour Colour::FromsRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Colour out;

	out.m_srgb = { (double)r / 255., (double)g / 255., (double)b / 255. };
	out.m_alpha = (double)a / 255.;

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
