#include "Image.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../ext/stb/stb_image.h"
#include "../../ext/stb/stb_image_write.h"

Image::Image() {
	m_h = 0;
	m_w = 0;
	m_channels = 0;
	m_size = 0;
	m_data = nullptr;
}

Image::Image(const char* file, const int forceChannels) {
	// temp
	m_h = 0;
	m_w = 0;
	m_channels = 0;
	m_size = 0;
	m_data = nullptr;
}

Image::Image(const Image& other) {
	//stbi_image_free(m_data);

	m_w = other.m_w;
	m_h = other.m_h;
	m_channels = other.m_channels;
	m_size = other.m_size;

	m_data = new uint8_t[m_size];

	memcpy(m_data, other.m_data, m_size);
}

Image::Image(const int w, const int h, const int channels) {
	m_w = w;
	m_h = h;
	m_channels = channels;
	m_size = (size_t)(m_w * m_h * m_channels);

	m_data = new uint8_t[m_size];
}

Image::~Image() {
	stbi_image_free(m_data);
}

Image& Image::operator=(const Image& other) {
	if (&other == this) return *this;

	stbi_image_free(m_data);

	m_w = other.m_w;
	m_h = other.m_h;
	m_channels = other.m_channels;
	m_size = other.m_size;

	m_data = new uint8_t[m_size];

	memcpy(m_data, other.m_data, m_size);

	return *this; if (&other == this) return *this;

	stbi_image_free(m_data);

	m_w = other.m_w;
	m_h = other.m_h;
	m_channels = other.m_channels;
	m_size = other.m_size;

	m_data = new uint8_t[m_size];

	memcpy(m_data, other.m_data, m_size);

	return *this;
}

Image::ImageType Image::GetFileType(const char* file) {
	const char* ext = strrchr(file, '.');

	if (ext != nullptr) {
		if (strcmp(ext, ".png") == 0) {
			return ImageType::PNG;
		} else if (strcmp(ext, ".jpg") == 0 || 
			strcmp(ext, ".jpeg") == 0 || 
			strcmp(ext, ".jpe") == 0 ||
			strcmp(ext, ".jif") == 0 ||
			strcmp(ext, ".jfif") == 0 ||
			strcmp(ext, ".jfi") == 0){
			return ImageType::JPG;
		} else if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".dib") == 0) {
			return ImageType::BMP;
		} else if (strcmp(ext, ".tga") == 0 ||
			strcmp(ext, ".icb") == 0 ||
			strcmp(ext, ".vda") == 0 ||
			strcmp(ext, ".vst") == 0) {
			return ImageType::TGA;
		}
	}

	return ImageType::NA;
}
