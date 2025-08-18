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
			m_size++;

			Colour col = Colour::FromHex(hex.c_str());
			m_colours.push_back(col);

			std::string hexOut = '#' + hex;
			std::string rgbOut = "rgb(" + col.sRGBUintDebug() + ')';
			std::string labOut = "oklab(" + col.OkLabDebug() + ')';

			Log::WriteOneLine(hexOut + " - " + rgbOut + " - " + labOut);
		}
	}
	Log::WriteOneLine("Palette Size: " + Log::ToString(m_size));
}

Palette::~Palette() {
}
