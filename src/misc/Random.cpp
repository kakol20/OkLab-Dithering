#include "Random.h"

#include <chrono>

unsigned int Random::Seed = 1;
unsigned int Random::MaxRand = ~0;

unsigned int Random::Rand() {
	//unsigned int state = Random::Seed * (unsigned int)747796405 + (unsigned int)2891336453;
	//unsigned int word = ((state >> ((state >> 28) + 4)) ^ state) * (unsigned int)27780373;

	//Random::Seed = (word >> 22) ^ word;
	//return Random::Seed;
	Random::Seed = Random::Seed * 747796405u + 2891336453u;
	unsigned int state = Random::Seed;
	unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

unsigned int Random::RandUInt(unsigned int min, unsigned int max) {
	if (min == max) return min;
	if (min > max) {
		min = min ^ max;
		max = min ^ max;
		min = min ^ max;
	}

	const unsigned int range = (max - min) + 1;
	const unsigned int threshold = Random::MaxRand - (Random::MaxRand % range);

	unsigned int out = 0;
	do {
		out = Random::Rand();
	} while (out > threshold);
	return (out % range) + min;
}

double Random::RandDouble(double min, double max) {
	//const unsigned int RAND_MAX = ~0;
	const unsigned int rand = Random::Rand();

	const double range = max - min;

	return ((double(rand) * (1. / double(Random::MaxRand))) * range) + min;
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
