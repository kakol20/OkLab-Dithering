#pragma once

class Random {
public:
	Random() {};
	~Random() {};

	static unsigned int Rand();
	static unsigned int RandUInt(unsigned int min, unsigned int max);

	static double RandDouble(const double min = 0., const double max = 1.);

	/// <summary>
	/// Sets seed to time since epoch
	/// </summary>
	static void EpochSeed();

	static unsigned int Seed;

private:
	static unsigned int MaxRand;
};