#pragma once
#include <cmath>
#include <vector>
#include <cstdint>

class BN_Helper {
public:
	BN_Helper() {}
	~BN_Helper() {}

	static std::vector<uint32_t> Generate(int N, uint32_t seed = 12345);

	static std::vector<uint32_t> GetMap(int N);
};

