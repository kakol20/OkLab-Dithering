#ifdef DEV_MODE

#include "DevTools.h"

#include "../image/Image.h"
#include <cstdint>

//void DevTools::GenerateGSTiles() {
//}

void DevTools::GenerateGSTiles() {
	//// https://github.com/Calinou/free-blue-noise-textures
	//Image blueNoise("dev/32_LDR_LLL1_0.png");
	//Log::WriteOneLine("Grayscale: " + Log::ToString(blueNoise.IsGrayscale()));

	//Log::Write("\n{\n");
	//for (int y = 0; y < blueNoise.GetHeight(); ++y) {
	//	Log::Write("\t");
	//	for (int x = 0; x < blueNoise.GetWidth(); ++x) {
	//		const uint8_t val = blueNoise.GetData(blueNoise.GetIndex(x, y));
	//		const std::string valStr = Log::ToString((unsigned int)val, 3, ' ');
	//		Log::Write(valStr + ", ");
	//	}
	//	Log::EndLine();
	//}
	//Log::Write("};\n");

	Image img(301, 301, 3);
	for (int x = 0; x < 301; ++x) {
		for (int y = 0; y < 301; ++y) {
			int tileX = x / 19;
			int tileY = y / 19;

			uint8_t gray = 0;

			if (x % 19 >= 16 || y % 19 >= 16) {
				gray = 0;
			} else {
				gray = static_cast<uint8_t>(tileY * 16 + tileX);
			}

			size_t index = img.GetIndex(x, y);

			img.SetData(index + 0, gray);
			img.SetData(index + 1, gray);
			img.SetData(index + 2, gray);
		}
	}
	img.Write("data/gs-tiles.png");
}
#endif // DEV_MODE
