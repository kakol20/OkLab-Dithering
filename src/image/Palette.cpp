#include "Palette.h"
#include <string>
#include <fstream>

#include "../wrapper/Log.h"

Palette::Palette(const char* file) {
	std::fstream p(file);

	m_size = 0;
	if (p) {
		std::string hex;
		while (std::getline(p, hex)) {
			if (hex.size() < 6) {
				Log::WriteOneLine(Log::LeadingCharacter(hex, 6, ' ') + ": is not a valid colour code");
				break;
			}
			hex.resize(6);
			++m_size;

			m_colours.emplace_back(hex.c_str());

			std::string hexOut = '#' + hex;
			std::string rgbOut = "rgb(" + m_colours.back().sRGBUintDebug() + ')';
			std::string labOut = "oklab(" + m_colours.back().OkLabDebug() + ')';

			Log::WriteOneLine(hexOut + " - " + rgbOut + " - " + labOut);
		}
	}
	Log::WriteOneLine("Palette Size: " + Log::ToString(m_size));
}

Palette::~Palette() {
}
