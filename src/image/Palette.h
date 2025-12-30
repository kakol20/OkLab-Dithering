#pragma once
#include "Colour.h"
#include <vector>

class Palette {
public:
	Palette(const char* file);
	~Palette();

	size_t size() const { return m_size; };

	Colour GetIndex(const size_t index) const { return m_colours[index]; };

	void CalculateAverageSpread();
	Colour GetAverageSpread() const { return m_avgSpread; };

private:
	std::vector<Colour> m_colours;
	size_t m_size;

	// Average distance between 1 colour and its closest neighbour
	Colour m_avgSpread;

};

