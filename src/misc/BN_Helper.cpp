#include "BN_Helper.h"

#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include "../wrapper/Log.h"

std::vector<uint32_t> BN_Helper::Generate(int N, uint32_t seed) {
	const int total = N * N;

	std::vector<bool> pattern(total, false);
	std::vector<double> energy(total, 0.0);
	std::vector<uint32_t> threshold(total, 0);

	std::mt19937 rng(seed);
	std::uniform_real_distribution<double> rand01(0.0, 1.0);

	// --------------------------------------------------------
	// Step 1: Initial random sparse pattern (~10%)
	// --------------------------------------------------------
	Log::WriteOneLine("Step 1: Initial random sparse pattern (~10%)");
	Log::StartTime();
	for (int i = 0; i < total; ++i) {
		if (rand01(rng) < 0.1)
			pattern[i] = true;

		Log::DebugProgress((double)i, (double)total - 1., 5.);
	}

	double sigma = N / 6.0;

	// --------------------------------------------------------
	// Step 2: Void-and-cluster iterations
	// --------------------------------------------------------

	const int iterations = total * 2;

	Log::WriteOneLine("Step 2: Void-and-cluster iterations");
	Log::StartTime();
	for (int iter = 0; iter < iterations; ++iter) {
		ComputeEnergy(pattern, energy, N, sigma);

		// --- Find densest active pixel (cluster)
		int removeIdx = -1;
		double maxEnergy = -1.0;

		for (int i = 0; i < total; ++i) {
			if (pattern[i] && energy[i] > maxEnergy) {
				maxEnergy = energy[i];
				removeIdx = i;
			}
		}

		if (removeIdx == -1) break;

		pattern[removeIdx] = false;

		ComputeEnergy(pattern, energy, N, sigma);

		// --- Find largest void (lowest energy inactive pixel)
		int addIdx = -1;
		double minEnergy = std::numeric_limits<double>::infinity();

		for (int i = 0; i < total; ++i) {
			if (!pattern[i] && energy[i] < minEnergy) {
				minEnergy = energy[i];
				addIdx = i;
			}

			Log::DebugProgress((double)iter + (double(i) / double(total)), (double)iterations - 1., 5.);
		}

		if (addIdx == -1) break;

		pattern[addIdx] = true;
	}

	// --------------------------------------------------------
	// Step 3: Ranking → threshold map
	// --------------------------------------------------------
	std::vector<bool> tempPattern = pattern;

	int rank = total - 1;

	Log::WriteOneLine("Step 3: Ranking -> threshold map");
	Log::StartTime();
	//while (rank >= 0) {
	//	ComputeEnergy(tempPattern, energy, N, sigma);

	//	int idx = -1;

	//	if (std::any_of(tempPattern.begin(), tempPattern.end(), [](bool v) { return v; })) {
	//		// Remove densest cluster
	//		double maxEnergy = -1.0;

	//		for (int i = 0; i < total; ++i) {
	//			if (tempPattern[i] && energy[i] > maxEnergy) {
	//				maxEnergy = energy[i];
	//				idx = i;
	//			}
	//		}

	//		tempPattern[idx] = false;
	//	} else {
	//		// Fill largest void
	//		double minEnergy = std::numeric_limits<double>::infinity();

	//		for (int i = 0; i < total; ++i) {
	//			if (!tempPattern[i] && energy[i] < minEnergy) {
	//				minEnergy = energy[i];
	//				idx = i;
	//			}
	//		}

	//		tempPattern[idx] = true;
	//	}

	//	if (idx == -1) break;

	//	threshold[idx] = rank;

	//	Log::DebugProgress(double(total - rank), double(total), 5.);

	//	--rank;
	//}

	for (int rank = total - 1; rank >= 0; --rank) {

		ComputeEnergy(tempPattern, energy, N, sigma);

		int idx = -1;
		double maxEnergy = -1.0;

		// ALWAYS remove most clustered active pixel
		for (int i = 0; i < total; ++i) {
			if (tempPattern[i] && energy[i] > maxEnergy) {
				maxEnergy = energy[i];
				idx = i;
			}
		}

		if (idx == -1) {
			// safety: fallback to any remaining pixel
			for (int i = 0; i < total; ++i) {
				if (tempPattern[i]) {
					idx = i;
					break;
				}
			}
		}

		if (idx == -1) break;

		threshold[idx] = rank;
		tempPattern[idx] = false;
	}

	return threshold;
}

int BN_Helper::ToroidalDist2(int x1, int y1, int x2, int y2, int N) {
	int dx = std::abs(x1 - x2);
	int dy = std::abs(y1 - y2);

	dx = std::min(dx, N - dx);
	dy = std::min(dy, N - dy);

	return dx * dx + dy * dy;
}

void BN_Helper::ComputeEnergy(const std::vector<bool>& pattern, std::vector<double>& energy, int N, double sigma) {
	const int total = N * N;

	std::fill(energy.begin(), energy.end(), 0.0);

	for (int y = 0; y < N; ++y) {
		for (int x = 0; x < N; ++x) {

			double e = 0.0;

			for (int yy = 0; yy < N; ++yy) {
				for (int xx = 0; xx < N; ++xx) {
					if (!pattern[static_cast<size_t>(yy) * N + xx]) continue;

					int d2 = ToroidalDist2(x, y, xx, yy, N);
					e += GaussianFromDist2(d2, sigma);
				}
			}

			energy[static_cast<size_t>(y) * N + x] = e;
		}
	}
}
