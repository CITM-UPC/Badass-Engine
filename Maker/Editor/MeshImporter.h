#pragma once

#include "../Engine/Mesh.h"
#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include "../Engine/Log.h"
#include <map>
#include <filesystem>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include "TextureImporter.h"
#include <string>
#include "../Engine/GameObject.h"
using namespace std;
namespace fs = std::filesystem;

class MeshImporter
{
    TextureImporter textureImporter;

public:
    std::vector<std::shared_ptr<GameObject>> meshGameObjects;
	vec3 _translation;
	vec3 _scale;
    glm::quat _rotation;
    
    std::vector<std::shared_ptr<Mesh>> ImportMesh(const aiScene& scene);
	std::vector<std::shared_ptr<Material>> createMaterialsFromFBX(const aiScene& scene, const std::filesystem::path& basePath);
    GameObject gameObjectFromNode(const aiScene& scene, const aiNode& node, const vector<shared_ptr<Mesh>>& meshes, const vector<shared_ptr<Material>>& materials);

    void SaveMeshToFile(const std::vector<std::shared_ptr<Mesh>>& gameObjects, const std::string& filePath);
    std::vector<std::shared_ptr<Mesh>> LoadMeshFromFile(const std::string& filePath, std::string& fbxPath);
	std::string GetFBXPath(const std::string& filePath);
    static void saveAsCustomFormat(const GameObject& gameObject, const std::string& outputPath);
    static GameObject loadCustomFormat(const std::string& path);
    void SaveMeshToFile(const std::vector<std::shared_ptr<GameObject>>& gameObjects, const std::string& filePath);
    std::vector<std::shared_ptr<GameObject>> LoadMeshFromFile(const std::string& filePath);
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Mesh>& mesh);
std::istream& operator>>(std::istream& is, std::vector<std::shared_ptr<Mesh>>& meshes);




