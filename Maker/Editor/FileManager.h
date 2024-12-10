#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "../Engine/Mesh.h"
#include "../Engine/GameObject.h"
#include "../Engine/Scene.h"
#include "MeshImporter.h"
#include "TextureImporter.h"

class FileManager
{
	MeshImporter meshImporter;
	TextureImporter textureImporter;

public:

	GameObject LoadFile(const char* path);
	void ImportTexture(const char* path);
	void LoadTexture(const char* path, GameObject& go);
	std::string getFileExtension(const std::string& filePath);
	std::string getFileNameWithoutExtension(const std::string& filePath);
	void LoadCustomFile(const char* path, GameObject& go);
	
};

