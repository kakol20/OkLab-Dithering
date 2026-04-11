#include "Random.h"

#include <chrono>
#include <cstdint>

uint32_t Random::Seed = 1;
uint32_t Random::MaxRand = ~0;

uint32_t Random::Rand() {
	//unsigned int state = Random::Seed * (unsigned int)747796405 + (unsigned int)2891336453;
	//unsigned int word = ((state >> ((state >> 28) + 4)) ^ state) * (unsigned int)27780373;

	//Random::Seed = (word >> 22) ^ word;
	//return Random::Seed;
	Random::Seed = Random::Seed * 747796405u + 2891336453u;
	uint32_t state = Random::Seed;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uint32_t Random::RandUInt(uint32_t min, uint32_t max) {
	if (min == max) return min;
	if (min > max) {
		min = min ^ max;
		max = min ^ max;
		min = min ^ max;
	}

	const uint32_t range = (max - min) + 1;
	const uint32_t threshold = Random::MaxRand - (Random::MaxRand % range);

	uint32_t out = 0;
	do {
		out = Random::Rand();
	} while (out > threshold);
	return (out % range) + min;
}

double Random::RandDouble(double min, double max) {
	//const unsigned int RAND_MAX = ~0;
	const uint32_t rand = Random::Rand();

	const double range = max - min;

	return ((double(rand) * (1. / double(Random::MaxRand))) * range) + min;
}

void Random::EpochSeed() {
	auto now = std::chrono::system_clock::now();
	auto epoch = now.time_since_epoch();
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

	Random::Seed = (uint32_t)sec;

#ifdef _DEBUG
	{
		bool breakpoint = true;
	}
#endif // _DEBUG
}
