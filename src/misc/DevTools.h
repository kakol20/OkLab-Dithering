#pragma once
#ifdef DEV_MODE

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
};


#endif // DEV_MODE