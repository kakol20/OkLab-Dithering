#include "../wrapper/Log.h"
#include "Colour.h"
#include "Palette.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <ios>
#include <ostream>

Palette::Palette() {
	m_size = 0;
}

Palette::Palette(const char* file, const bool grayscale) {
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

			bool push = true;

			Colour col(hex.c_str());

			if (grayscale && !col.IsGrayscale()) push = false;

			//Colour col(hex.c_str());
			//m_colours.push_back(col);
			
			if (push) m_colours.emplace_back(hex.c_str());
		}

		Log::WriteOneLine("Palette Size: " + Log::ToString(m_size, 0, '0'));

		//Colour::SetMathMode(Colour::MathMode::OkLCh);

		std::sort(m_colours.begin(), m_colours.end());

		for (auto it = m_colours.begin(); it != m_colours.end(); ++it) {
			std::string hexOut = "  #" + it->GetHex();
			std::string rgbOut = "rgb(" + it->sRGBUintDebug() + ')';
			std::string labOut = "oklab(" + it->OkLabDebug() + ')';
			std::string lchOut = "oklch(" + it->OkLChDebug() + ')';

			Log::WriteOneLine(hexOut + " - " + rgbOut + " - " + labOut + " - " + lchOut);
		}
	}
}

Palette::~Palette() {
}

void Palette::CalculateAverageSpread() {
	// Memoisastion of spreads between colours
	std::unordered_map<size_t, Colour> mem;

	// size_t(x + y * imgWidth);
	size_t count = 0;

	m_avgSpread.PureBlack();
	for (size_t i = 0; i < m_size; ++i) {
		for (size_t j = 0; j < m_size; ++j) {
			if (i == j) continue;

			// Makes sure that two pairs of indices query from the same key
			const size_t smaller = i < j ? i : j;
			const size_t bigger = i > j ? i : j;
			const size_t key = smaller + bigger * m_size;

			if (mem.find(key) != mem.end()) continue;

			++count;

			Colour spread = m_colours[i] - m_colours[j];
			spread.Abs();

			mem[key] = spread;

			m_avgSpread += spread;
		}
	}
	m_avgSpread *= 1. / static_cast<double>(count);
	m_avgSpread.Update();
}

void Palette::Save(const char* file) {
	const std::filesystem::path p = file;
	const std::filesystem::path dir = p.parent_path();
	if (!p.parent_path().empty() && !std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	std::fstream output;
	output.open(file, std::ios_base::out);

	for (size_t i = 0; i < m_colours.size(); ++i) {
		output << m_colours[i].GetHex();

		if (i + 1 < m_colours.size()) output << '\n';
	}
	output << std::flush;
}

void Palette::SetToNearestUint() {
	for (size_t i = 0; i < m_colours.size(); ++i) {
		m_colours[i] = Colour::FromHex(m_colours[i].GetHex().c_str());
	}
}

void Palette::UpdateEveryCol() {
	for (size_t i = 0; i < m_colours.size(); ++i) {
		m_colours[i].Update();
	}
}
