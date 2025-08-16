#pragma once
#include <cstdint>

class Image {
public:
	Image();
	Image(const char* file, const int forceChannels = 0);
	Image(const Image& other);
	Image(const int w, const int h, const int channels);
	~Image();

	Image& operator=(const Image& other);

	static enum class ImageType {
		PNG, JPG, BMP, TGA, NA
	};
	static Image::ImageType GetFileType(const char* file);

private:
	uint8_t* m_data;
	size_t m_size = 0;
	int m_w, m_h, m_channels;
};

