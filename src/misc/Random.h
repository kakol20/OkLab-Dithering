#pragma once
#include <cstdint>

class Random {
public:
	Random() {};
	~Random() {};

	static uint32_t Rand();
	static uint32_t RandUInt(uint32_t min, uint32_t max);

	static double RandDouble(const double min = 0., const double max = 1.);

	/// <summary>
	/// Sets seed to time since epoch
	/// </summary>
	static void EpochSeed();

	static uint32_t Seed;

private:
	static uint32_t MaxRand;
};