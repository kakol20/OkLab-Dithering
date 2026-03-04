#include "Random.h"

#include <chrono>

unsigned int Random::Seed = 1;
unsigned int Random::MaxRand = ~0;

unsigned int Random::Rand() {
	unsigned int state = Random::Seed * (unsigned int)747796405 + (unsigned int)2891336453;
	unsigned int word = ((state >> ((state >> 28) + 4)) ^ state) * (unsigned int)27780373;

	Random::Seed = (word >> 22) ^ word;
	return Random::Seed;
}

unsigned int Random::RandUInt(unsigned int min, unsigned int max) {
	if (min == max) return min;
	if (min > max) {
		min = min ^ max;
		max = min ^ max;
		min = min ^ max;
	}

	const unsigned int delta = (max - min) + 1;
	return (Random::Rand() % delta) + min;
}

double Random::RandDouble(double min, double max) {
	//const unsigned int RAND_MAX = ~0;
	const unsigned int rand = Random::Rand();

	const double range = max - min;

	return ((double(rand) / double(Random::MaxRand)) * range) + min;
}

void Random::EpochSeed() {
	auto now = std::chrono::system_clock::now();
	auto epoch = now.time_since_epoch();
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

	Random::Seed = (unsigned int)sec;

#ifdef _DEBUG
	{
		bool breakpoint = true;
	}
#endif // _DEBUG
}
