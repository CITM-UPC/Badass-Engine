#include "TextureImporter.h"


std::shared_ptr<Image> TextureImporter::ImportTexture(const std::string& pathFile)
{
	auto texture = std::make_shared<Image>();
	auto img = ilGenImage();
	ilBindImage(img);
	ilLoadImage((const wchar_t*)pathFile.c_str());
	auto width = ilGetInteger(IL_IMAGE_WIDTH);

	auto height = ilGetInteger(IL_IMAGE_HEIGHT);

	auto channels = ilGetInteger(IL_IMAGE_CHANNELS);
	auto data = ilGetData();

	//load image as a texture in VRAM
	texture->load(width, height, channels, data);

	//now we can delete image from RAM
	ilDeleteImage(img);
	return texture;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Image>& tex)
{
	if (!tex) {
		return os;
	}

	os << tex->width() << " " << tex->height() << " " << tex->channels() << " ";
	size_t dataSize = tex->width() * tex->height() * tex->channels();
	for (size_t i = 0; i < dataSize; ++i)
	{
		os << static_cast<int>(tex->data()[i]) << " ";
	}
	return os;
}

std::istream& operator>>(std::istream& is, std::shared_ptr<Image>& tex)
{
    if (!tex) {
        tex = std::make_shared<Image>();
    }

    int width, height, channels;
    is >> width >> height >> channels;
    size_t dataSize = width * height * channels;
    std::vector<unsigned char> data(dataSize);
    for (size_t i = 0; i < dataSize; ++i)
    {
        int value;
        is >> value;
        data[i] = static_cast<unsigned char>(value);
    }
    tex->load(width, height, channels, data.data());
    return is;
}
