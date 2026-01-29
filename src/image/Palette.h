#pragma once
#include "Colour.h"
#include <vector>

class Palette {
public:
	Palette(const char* file);
	~Palette();

	size_t size() const { return m_size; };

	Colour GetColour(const size_t index) const { return m_colours[index]; };

	/// <summary>
	/// Average spread between one colour and other colours
	/// </summary>
	void CalculateAverageSpread();

	/// <summary>
	/// Average spread between one colour and other colours
	/// </summary>
	Colour GetAverageSpread() const { return m_avgSpread; };

private:
	std::vector<Colour> m_colours;
	size_t m_size;

	Colour m_avgSpread;

};

