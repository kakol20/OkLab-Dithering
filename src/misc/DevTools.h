#pragma once
#ifdef DEV_MODE

class DevTools {
public:
	DevTools() {};
	~DevTools() {};

	static void Run();

private:
	static void GenerateBlueNoise();
	static void GenerateGSTiles();
	static void PaletteValues();
};


#endif // DEV_MODE