#pragma once
#include <cmath>
#include <vector>
#include <cstdint>

class BN_Helper {
public:
	BN_Helper() {}
	~BN_Helper() {}

	static std::vector<uint32_t> Generate(int N, uint32_t seed = 12345);

private:
	/// <summary>
	/// Toroidal squared distance (no sqrt needed)
	/// </summary>
	/// <param name="x1"></param>
	/// <param name="y1"></param>
	/// <param name="x2"></param>
	/// <param name="y2"></param>
	/// <param name="N"></param>
	/// <returns></returns>
	static int ToroidalDist2(int x1, int y1, int x2, int y2, int N);

	/// <summary>
	/// Gaussian kernel using squared distance
	/// </summary>
	/// <param name="dist2"></param>
	/// <param name="sigma"></param>
	/// <returns></returns>
	static inline double GaussianFromDist2(int dist2, double sigma) {
		return std::exp(-(double)dist2 / (2.0 * sigma * sigma));
	}

	/// <summary>
	/// Compute full energy map
	/// </summary>
	/// <param name="pattern"></param>
	/// <param name="energy"></param>
	/// <param name="N"></param>
	/// <param name="sigma"></param>
	static void ComputeEnergy(
		const std::vector<bool>& pattern,
		std::vector<double>& energy,
		int N,
		double sigma
	);
};

