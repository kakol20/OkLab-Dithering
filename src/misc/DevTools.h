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

	static void GenerateBlueNoise();

	static void Misc();
};


#endif // DEV_MODE