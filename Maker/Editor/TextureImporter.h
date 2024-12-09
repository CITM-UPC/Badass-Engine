#pragma once

#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include "../Engine/Log.h"
#include "../Engine/Image.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

class TextureImporter
{
public:

    


	std::shared_ptr<Image> ImportTexture(const std::string& pathFile);

	void SaveTextureToFile(const std::shared_ptr<Image>& texture, const std::string& filePath);
	std::shared_ptr<Image> LoadTextureFromFile(const std::string& filePath);
};

GLenum formatFromChannels(unsigned char channels);

struct ImageDTO {
    unsigned short width = 0;
    unsigned short height = 0;
    unsigned char channels = 0;
    std::vector<char> data;

    ImageDTO() = default;

    explicit ImageDTO(const std::shared_ptr<Image>& img) :
        width(img->width()),
        height(img->height()),
        channels(img->channels()), data(width* height* channels) {

        img->bind();
        glGetTexImage(GL_TEXTURE_2D, 0, formatFromChannels(img->channels()), GL_UNSIGNED_BYTE, data.data());
    }
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Image>& tex);
std::istream& operator>>(std::istream& is, std::shared_ptr<Image>& tex);



