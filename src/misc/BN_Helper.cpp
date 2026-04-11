#include "BN_Helper.h"

#include "../../ext/bluenoise/noise2d.h"
#include "../../res/resource.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <windows.h>

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
	int res = IDI_BN16; // default

	if (N == 32) res = IDI_BN32;
	if (N == 64) res = IDI_BN64;
	if (N == 128) res = IDI_BN128;

	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(res), RT_RCDATA);
	if (!hRes) return;

	HGLOBAL hData = LoadResource(NULL, hRes);
	void* pData = LockResource(hData);
	DWORD dataSize = SizeofResource(NULL, hRes);
	if (!pData || dataSize < sizeof(uint32_t)) return;

	const char* bytes = reinterpret_cast<const char*>(pData);

	// Read the size first
	uint32_t size = 0;
	std::memcpy(&size, bytes, sizeof(uint32_t));

	// Check that the resource size matches expected
	if (dataSize < sizeof(uint32_t) + size * sizeof(uint32_t)) return;

	std::vector<uint32_t> result(size);
	std::memcpy(result.data(), bytes + sizeof(uint32_t), size * sizeof(uint32_t));

	return result;
}
