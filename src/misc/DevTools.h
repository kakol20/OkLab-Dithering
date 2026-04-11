#pragma once
#ifdef DEV_MODE

#include <cstdint>

class DevTools {
public:
	DevTools() {};
	~DevTools() {};

	static void Run();

private:
	static void GenerateGSTiles();
	static void PaletteValues();

	// Generate a palette using blue noise
	static void GenerateBlueNoisePalette();

	static void ThresholdToImage();

	static void Misc();

	static void DebugThreshold();

	static void GenerateBlueNoise(const uint32_t size, const char* filename);
	static void ReadBlueNoiseBin(const int res);
};


#endif // DEV_MODE