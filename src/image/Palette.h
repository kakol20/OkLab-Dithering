#pragma once
#include <vector>

#include "Colour.h"

class Palette {
public:
	Palette(const char* file);
	~Palette();

	size_t size() const { return m_size; };

	const Colour& operator[](const size_t index) { return m_colours[index]; };

private:
	std::vector<Colour> m_colours;
	size_t m_size;

};

