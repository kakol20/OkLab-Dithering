#pragma once
#include "Colour.h"
#include <vector>
#include <utility>
#include <algorithm>

class Palette {
public:

	/// <summary>
	/// Create Empty Palette
	/// </summary>
	Palette();

	Palette(const char* file, const bool grayscale = false);
	~Palette();

	size_t size() const { return m_size; };

	const Colour& GetColour(const size_t index) const { return m_colours[index]; };

	/// <summary>
	/// Average spread between one colour and other colours
	/// </summary>
	void CalculateAverageSpread();

	/// <summary>
	/// Average spread between one colour and other colours
	/// </summary>
	const Colour& GetAverageSpread() const { return m_avgSpread; }

	void Save(const char* file);

	const Colour& back() const { return m_colours[m_size - 1]; }
	const Colour& front() const { return m_colours[0]; }
	
	template<typename... Args>
	Colour& emplace_back(Args&&... args) {
		return m_colours.emplace_back(std::forward<Args>(args)...);
	}

	void Sort() { std::sort(m_colours.begin(), m_colours.end()); }

	void SetToNearestUint();
	void UpdateEveryCol();

private:
	std::vector<Colour> m_colours;
	size_t m_size;

	Colour m_avgSpread;

};

