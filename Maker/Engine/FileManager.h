#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "GameObject.h"
#include "Scene.h"

class FileManager
{

public:

	void LoadFile(const char* path, GameObject& go);
	void LoadTexture(const char* path, GameObject& go);
	std::string getFileExtension(const std::string& filePath);
	
};

