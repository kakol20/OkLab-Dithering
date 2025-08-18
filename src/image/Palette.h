#pragma once
#include <vector>

#include "Colour.h"

class Palette {
public:
	Palette(const char* file);
	~Palette();

	size_t size() const { return m_size; };

private:
	std::vector<Colour> m_colours;
	size_t m_size;

};

