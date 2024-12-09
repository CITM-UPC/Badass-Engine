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
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Image>& tex);
std::istream& operator>>(std::istream& is, std::shared_ptr<Image>& tex);

