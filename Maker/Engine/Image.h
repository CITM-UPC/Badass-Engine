#pragma once

#include <ostream>
#include <istream>
#include <vector>
#include <IL/il.h>
#include <IL/ilu.h>
#include <glm/glm.hpp>

class Image {

	unsigned int _id = 0;
	unsigned short _width = 0;
	unsigned short _height = 0;
	unsigned char _channels = 0;
	std::unique_ptr<char[]> _data;

	mutable std::vector<unsigned char> _dataCache;

public:
	unsigned int id() const { return _id; }
	auto width() const { return _width; }
	auto height() const { return _height; }
	auto channels() const { return _channels; }
	const char* data() const { return _data.get(); }
	char* data() { return _data.get(); }

	Image() = default;
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&& other) noexcept;
	Image& operator=(Image&& other) noexcept = delete;
	~Image();

	void bind() const;
	void load(int width, int height, int channels, void* data);
	// Load Texture
	void LoadTexture(const std::string& path);

	const std::vector<unsigned char>& rawData() const;
};

std::ostream& operator<<(std::ostream& os, const Image& img);
std::istream& operator>>(std::istream& is, Image& img);

