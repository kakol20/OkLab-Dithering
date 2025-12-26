#include "Image.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../ext/stb/stb_image.h"
#include "../../ext/stb/stb_image_write.h"

#include "../wrapper/Log.h"

Image::Image() {
	m_h = 0;
	m_w = 0;
	m_channels = 0;
	m_size = 0;
	m_data = nullptr;
}

Image::Image(const char* file, const int forceChannels) {
	Image::Read(file, forceChannels);
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
			strcmp(ext, ".jfi") == 0) {
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

bool Image::Read(const char* file, const int forceChannels) {
	if (GetFileType(file) == ImageType::NA) {
		Log::WriteOneLine("Invalid file type");
		return false;
	}

	//Log::StartLine();
	if (forceChannels > 0 && forceChannels <= 4) {
		m_data = stbi_load(file, &m_w, &m_h, &m_channels, forceChannels);
	} else {
		m_data = stbi_load(file, &m_w, &m_h, &m_channels, 0);
	}

	if (m_data != NULL) {
		Log::WriteOneLine("Read success ");
	} else {
		Log::WriteOneLine("Read failed ");
	}

	m_size = (size_t)(m_w * m_h * m_channels);

	Log::WriteOneLine(file);
	//Log::EndLine();

	return m_data != NULL;
}

bool Image::Write(const char* file) const {
	Image::ImageType type = Image::GetFileType(file);
	int success = 0;

	switch (type) {
	case Image::ImageType::PNG:
		success = stbi_write_png(file, m_w, m_h, m_channels, m_data, m_w * m_channels);
		break;
	case Image::ImageType::JPG:
		success = stbi_write_jpg(file, m_w, m_h, m_channels, m_data, 100);
		break;
	case Image::ImageType::BMP:
		success = stbi_write_bmp(file, m_w, m_h, m_channels, m_data);
		break;
	case Image::ImageType::TGA:
		success = stbi_write_tga(file, m_w, m_h, m_channels, m_data);
		break;
	case Image::ImageType::NA:
		//std::cout << "File type not supported\nPNG, JPG, BMP or TGA";
		Log::StartLine();
		Log::Write("File type not supported - PNG, JPG, BMP or TGA");
		Log::EndLine();
		success = 0;
		break;
	default:
		Log::StartLine();
		Log::Write("File type not supported - PNG, JPG, BMP or TGA");
		Log::EndLine();
		success = 0;
		break;
	}

	Log::StartLine();
	if (success != 0) {
		//std::cout << "Write success " << file << '\n';
		Log::Write("Write success ");
		Log::Write(file);
	} else {
		//std::cout << "Write fail " << file << '\n';
		Log::Write("Write fail ");
		Log::Write(file);
	}
	Log::EndLine();

	return success != 0;
}

size_t Image::GetIndex(const int x, const int y) const {
	if (x < 0 || x >= m_w || y < 0 || y >= m_h) return (size_t)NAN;
	return size_t((x + y * m_w) * m_channels);
}

size_t Image::GetIndex_s(const int x, const int y, const int width, const int channels) {
	return size_t((x + y * width) * channels);
}

void Image::Clear() {
	for (size_t i = 0; i < m_size; ++i) {
		m_data[i] = 0;
	}
}

void Image::HideSemiTransparent(const int threshold) {
	if (m_channels == 2 || m_channels == 4) {
		for (size_t i = (size_t)m_channels - 1; i < m_size; i += m_channels) {
			if (m_data[i] < threshold) {
				m_data[i] = 0;
			} else {
				m_data[i] = 255;
			}
		}
	}
}

bool Image::HasAlphaChannel() const {
	return m_channels == 4 || m_channels == 2;
}

void Image::AddAlphaChannel() {
	if (m_channels == 1 || m_channels == 3) {
		const int outChannels = m_channels + 1;
		const size_t outSize = (size_t)(m_w * m_h * outChannels);
		uint8_t* outData = new uint8_t[outSize];
		for (int i = 0; i < m_w * m_h; ++i) {
			for (int c = 0; c < m_channels; ++c) {
				outData[i * outChannels + c] = m_data[i * m_channels + c];
			}
			outData[i * outChannels + (outChannels - 1)] = 255;
		}
		stbi_image_free(m_data);
		m_channels = outChannels;
		m_size = outSize;
		m_data = new uint8_t[m_size];
		memcpy(m_data, outData, m_size);
		stbi_image_free(outData);
	}
}

void Image::ToRGB() {
	if (IsGrayscale()) {		
		const int outChannels = m_channels == 1 ? 3 : 4;
		const size_t outSize = (size_t)(m_w * m_h * outChannels);
		uint8_t* outData = new uint8_t[outSize];

		for (int i = 0; i < m_w * m_h; ++i) {
			outData[i * outChannels + 0] = m_data[i * m_channels];
			outData[i * outChannels + 1] = m_data[i * m_channels];
			outData[i * outChannels + 2] = m_data[i * m_channels];

			if (m_channels == 2) outData[i * outChannels + 3] = m_data[i * m_channels + 1];
		}

		stbi_image_free(m_data);
		m_channels = outChannels;
		m_size = outSize;
		m_data = new uint8_t[m_size];
		memcpy(m_data, outData, m_size);

		stbi_image_free(outData);
	}
}
