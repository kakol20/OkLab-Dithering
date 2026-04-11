#include "BN_Helper.h"

#include "../../ext/bluenoise/noise2d.h"
#include "../../res/resource.h"
#include "../wrapper/Log.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <random>
#include <vector>

std::vector<uint32_t> BN_Helper::Generate(int N, uint32_t seed) {
	Noise2D<uint32_t> noise_2D = Noise2D<uint32_t>(N, N, N * N);
	noise_2D.generate_blue_noise();

	std::vector<uint32_t> output(N * N, 0);
	for (int y = 0; y < N; ++y) {
		for (int x = 0; x < N; ++x) {
			output[static_cast<size_t>(x + y * N)] = noise_2D.get_noise_at(x, y);
		}
	}

	return output;
}

std::vector<uint32_t> BN_Helper::GetMap(int N) {
	int res = IDI_BN16;

	if (N == 32) res = IDI_BN16;

	return std::vector<uint32_t>();
}
