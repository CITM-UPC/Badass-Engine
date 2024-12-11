#include "TextureImporter.h"

std::string TextureImporter::getFileExtension(const std::string& filePath)
{
	// Find the last dot in the file path
	size_t dotPosition = filePath.rfind('.');

	// If no dot is found, return an empty string
	if (dotPosition == std::string::npos) {
		return "";
	}

	// Extract and return the file extension
	return filePath.substr(dotPosition + 1);
}

std::shared_ptr<Image> TextureImporter::ImportTexture(const std::string& pathFile)
{
	auto image = std::make_shared<Image>();
	auto img = ilGenImage();
	ilBindImage(img);
	ilLoadImage((const wchar_t*)pathFile.c_str());

	// Check if the image is a TGA file
	std::string extension = getFileExtension(pathFile);
	if (extension == "tga") {
		iluFlipImage(); // Flip the image vertically
	}

	auto width = ilGetInteger(IL_IMAGE_WIDTH);
	auto height = ilGetInteger(IL_IMAGE_HEIGHT);
	auto channels = ilGetInteger(IL_IMAGE_CHANNELS);
	auto data = ilGetData();

	// Load image as a texture in VRAM
	image->load(width, height, channels, data);

	// Now we can delete image from RAM
	ilDeleteImage(img);
	return image;
}

void TextureImporter::SaveTextureToFile(const std::shared_ptr<Image>& texture, const std::string& filePath)
{
	std::ofstream os(filePath, std::ios::binary);
	os << texture;
}

void TextureImporter::saveAsCustomImage(const std::shared_ptr<Image>& image, const std::string& outputPath) {
	if (!image) {
		throw std::runtime_error("No image to save.");
	}

	std::ofstream file(outputPath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file for saving: " + outputPath);
	}

	uint32_t width = image->width();
	uint32_t height = image->height();
	uint32_t channels = image->channels();

	// Serialize metadata
	file.write(reinterpret_cast<const char*>(&width), sizeof(width));
	file.write(reinterpret_cast<const char*>(&height), sizeof(height));
	file.write(reinterpret_cast<const char*>(&channels), sizeof(channels));

	// Serialize raw image data
	const auto& rawData = image->rawData();
	file.write(reinterpret_cast<const char*>(rawData.data()), rawData.size());

	file.close();
}

std::shared_ptr<Image> TextureImporter::LoadTextureFromFile(const std::string& filePath)
{
	std::ifstream is(filePath, std::ios::binary);
	std::shared_ptr<Image> texture = std::make_shared<Image>();
	is >> texture;
	return texture;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Image>& img)
{
	ImageDTO dto(img);
	os.write((const char*)&dto.width, sizeof(dto.width));
	os.write((const char*)&dto.height, sizeof(dto.height));
	os.write((const char*)&dto.channels, sizeof(dto.channels));
	os.write(dto.data.data(), dto.data.size());
	return os;

}

std::istream& operator>>(std::istream& is, std::shared_ptr<Image>& img)
{
	ImageDTO dto;
	is.read((char*)&dto.width, sizeof(dto.width));
	is.read((char*)&dto.height, sizeof(dto.height));
	is.read((char*)&dto.channels, sizeof(dto.channels));

	dto.data.resize(dto.width * dto.height * dto.channels);
	is.read(dto.data.data(), dto.data.size());

	img->load(dto.width, dto.height, dto.channels, dto.data.data());

	return is;

}

GLenum formatFromChannels(unsigned char channels)
{
	switch (channels) {
	case 1: return GL_LUMINANCE;
	case 2: return GL_LUMINANCE_ALPHA;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
	default: return GL_RGB;
	}
}
