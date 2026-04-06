#include <vector>
#include <random>
#include <limits>
#include <cmath>
#include <cstdint>

struct Point {
	int x, y;
};

// Squared toroidal distance (no sqrt)
inline int ToroidalDist2(const Point& a, const Point& b, int N) {
	int dx = std::abs(a.x - b.x);
	int dy = std::abs(a.y - b.y);

	dx = std::min(dx, N - dx);
	dy = std::min(dy, N - dy);

	return dx * dx + dy * dy;
}

// ------------------------------------------------------------
// Spatial grid for fast nearest neighbour lookup
// ------------------------------------------------------------
class Grid {
public:
	Grid(int N, int cellSize)
		: N(N), cellSize(cellSize)
	{
		gridDim = (N + cellSize - 1) / cellSize;
		cells.resize(gridDim * gridDim);
	}

	void Insert(const Point& p) {
		int gx = p.x / cellSize;
		int gy = p.y / cellSize;
		cells[gy * gridDim + gx].push_back(p);
	}

	// Find nearest distance squared using local cells
	int NearestDist2(const Point& c, int bestSoFar) const {
		int gx = c.x / cellSize;
		int gy = c.y / cellSize;

		int minDist2 = std::numeric_limits<int>::max();

		// Search neighbouring cells (3x3)
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = (gx + dx + gridDim) % gridDim;
				int ny = (gy + dy + gridDim) % gridDim;

				const auto& cell = cells[ny * gridDim + nx];

				for (const Point& p : cell) {
					int d2 = ToroidalDist2(c, p, N);

					if (d2 < minDist2) {
						minDist2 = d2;

						// Early exit if already worse than best candidate
						if (minDist2 < bestSoFar)
							return minDist2;
					}
				}
			}
		}

		return minDist2;
	}

private:
	int N;
	int cellSize;
	int gridDim;
	std::vector<std::vector<Point>> cells;
};

// ------------------------------------------------------------
// Generate blue noise threshold map
// ------------------------------------------------------------
std::vector<float> GenerateBlueNoise_Optimised(int N, int K = 16, uint32_t seed = 12345) {
	const int total = N * N;

	std::vector<float> threshold(total, 0.0f);
	std::vector<bool> used(total, false);
	std::vector<Point> points;
	points.reserve(total);

	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> dist(0, N - 1);

	// Heuristic grid cell size
	int cellSize = std::max(1, N / 16);
	Grid grid(N, cellSize);

	for (int i = 0; i < total; ++i) {
		Point bestCandidate{};
		int bestDist2 = -1;

		for (int k = 0; k < K; ++k) {
			Point candidate{ dist(rng), dist(rng) };
			int idx = candidate.y * N + candidate.x;

			// Avoid duplicates
			if (used[idx]) continue;

			int d2;

			if (points.empty()) {
				d2 = std::numeric_limits<int>::max();
			} else {
				d2 = grid.NearestDist2(candidate, bestDist2);
			}

			if (d2 > bestDist2) {
				bestDist2 = d2;
				bestCandidate = candidate;
			}
		}

		int index = bestCandidate.y * N + bestCandidate.x;

		// Fallback (very rare): find any unused pixel
		if (used[index]) {
			for (int j = 0; j < total; ++j) {
				if (!used[j]) {
					bestCandidate = { j % N, j / N };
					index = j;
					break;
				}
			}
		}

		used[index] = true;
		points.push_back(bestCandidate);
		grid.Insert(bestCandidate);

		threshold[index] = static_cast<float>(i) / static_cast<float>(total);
	}

	return threshold;
}