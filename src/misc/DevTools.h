#pragma once
#ifdef DEV_MODE

class DevTools {
public:
	DevTools() {};
	~DevTools() {};

	static void Run();

private:
	static void GenerateGSTiles();
	static void GenerateBlueNoise();
};


#endif // DEV_MODE