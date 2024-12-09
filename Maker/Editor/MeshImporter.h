#pragma once

#include "../Engine/Mesh.h"
#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include "../Engine/Log.h"

class MeshImporter
{
public:

    std::shared_ptr<Mesh> ImportMesh(const char* pathFile);

    void SaveMeshToFile(const std::shared_ptr<Mesh>& mesh, const std::string& filePath);
    std::shared_ptr<Mesh> LoadMeshFromFile(const std::string& filePath);
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Mesh>& mesh);
std::istream& operator>>(std::istream& is, std::shared_ptr<Mesh>& mesh);




