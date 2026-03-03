#pragma once
#ifdef DEV_MODE

class DevTools {
public:
	DevTools() {};
	~DevTools() {};

	static void GenerateGSTiles();
};


#endif // DEV_MODE