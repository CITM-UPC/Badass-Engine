#include "SceneSerializator.h"
#include "Engine/GameObject.h"
#include "Engine/Mesh.h"
#include "Engine/Image.h"
#include "MeshImporter.h"
#include "TextureImporter.h"

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include "MyGui.h"
#include "MyGUI.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fs = std::filesystem;

std::string SceneManager::getFileDirectory(const std::string& filePath) {
    return std::filesystem::path(filePath).parent_path().string();
}

std::string SceneManager::getFileNameWithoutExtension(const std::string& filePath) {
    return std::filesystem::path(filePath).stem().string();
}

std::vector<GameObject> SceneManager::gameObjectsOnScene;

bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}
void SceneManager::LoadGameObject(const std::string& filePath) {
    auto mesh = std::make_shared<Mesh>();
    GameObject go;
    mesh->LoadFile(filePath.c_str());
    go.setMesh(mesh);

    // Check for associated texture
    std::string texturePath = filePath.substr(0, filePath.find_last_of('.')) + ".png";
    if (fileExists(texturePath)) {
        auto texture = TextureImporter::LoadTextureFromFile(texturePath);
        go.setTextureImage(texture);

        // Save the texture only if it exists
        std::string customImagePath = filePath.substr(0, filePath.find_last_of('.')) + ".customimage";
        TextureImporter::SaveTextureToFile(texture, customImagePath);
    }

    go.SetName("GameObject (" + std::to_string(gameObjectsOnScene.size()) + ")");
    gameObjectsOnScene.push_back(go);

    // Save the model in custom format using MeshImporter
    std::string customFilePath = filePath.substr(0, filePath.find_last_of('.')) + ".custom";
    MeshImporter::SaveMeshToFile({ mesh }, customFilePath);
}


GameObject* SceneManager::getGameObject(int index) {
    return &gameObjectsOnScene[index];
}

void SceneManager::deleteSelectedObject() {
    if (persistentSelectedGameObject) {
        auto it = std::find_if(gameObjectsOnScene.begin(), gameObjectsOnScene.end(),
            [](const GameObject& obj) { return &obj == persistentSelectedGameObject; });
        if (it != gameObjectsOnScene.end()) {
            gameObjectsOnScene.erase(it);
            persistentSelectedGameObject = nullptr;
      
        }
    }
}

void SceneManager::LoadCustomModel(const std::string& filePath) {
    try {
        auto mesh = std::make_shared<Mesh>();
        mesh->LoadFile(filePath.c_str());

        GameObject go;
        go.setMesh(mesh);
        go.SetName("Custom Model (" + std::to_string(gameObjectsOnScene.size()) + ")");

        // Check for associated texture
        std::string texturePath = filePath.substr(0, filePath.find_last_of('.')) + ".png";
        if (fileExists(texturePath)) {
            auto texture = TextureImporter::LoadTextureFromFile(texturePath);
            go.setTextureImage(texture);
        }

        gameObjectsOnScene.push_back(go);
    }
    catch (const std::exception& e) {
        // Handle exceptions if necessary
    }
}

void SceneManager::saveScene(const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::out);
    if (!outFile) {
        
        return;
    }

    outFile << "{\n\"GameObjects\": [\n";
    for (size_t i = 0; i < gameObjectsOnScene.size(); ++i) {
        const GameObject *go = &gameObjectsOnScene[i];
		const GameObject* parent = &go->parent();
        const auto& transform = go->GetComponent<TransformComponent>()->transform();

        outFile << "  {\n";
        outFile << "    \"Id\": " << go->getId() << ",\n";
        outFile << "    \"ParentId\": " << (&go->parent() ? parent->getId() : -1) << ",\n";
        outFile << "    \"Name\": \"" << go->GetName() << "\",\n";
        outFile << "    \"Transform\": {\n";
        outFile << "      \"Position\": [" << transform.pos().x << ", " << transform.pos().y << ", " << transform.pos().z << "],\n";
        outFile << "      \"Scale\": [" << transform.GetScale().x << ", "
            << transform.GetScale().y << ", "
            << transform.GetScale().z << "],\n";
        outFile << "      \"Rotation\": [" << transform.GetRotation().x << ", "
            << transform.GetRotation().y << ", "
            << transform.GetRotation().z << "]\n";
        outFile << "    }\n";
        outFile << "  }";
        if (i != gameObjectsOnScene.size() - 1) outFile << ",";
        outFile << "\n";
    }
    outFile << "]\n}";
    outFile.close();
}


void SceneManager::loadScene(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::in);
    if (!inFile) {
        
        return;
    }

    gameObjectsOnScene.clear();

    std::string line;
    while (std::getline(inFile, line)) {
        if (line.find("\"Id\":") != std::string::npos) {
            GameObject go;
            go.setId(std::stoi(line.substr(line.find(":") + 1)));

            // Parse Name
            std::getline(inFile, line);
            if (line.find("\"Name\":") != std::string::npos) {
                std::string name = line.substr(line.find(":") + 2);
                name.erase(name.find_last_of("\""));
                go.SetName(name);
            }

            // Parse Transform
            Transform transform;
            while (std::getline(inFile, line) && line.find("}") == std::string::npos) {
                if (line.find("\"Position\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 pos;
                    char delim;
                    iss >> pos.x >> delim >> pos.y >> delim >> pos.z;
                    transform.translate(pos);
                }
                else if (line.find("\"Scale\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 scale;
                    char delim;
                    iss >> scale.x >> delim >> scale.y >> delim >> scale.z;
                    transform.SetScale(scale);
                }
                else if (line.find("\"Rotation\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 rotation;
                    char delim;
                    iss >> rotation.x >> delim >> rotation.y >> delim >> rotation.z;
                    glm::quat rotQuat = glm::quat(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));
                    glm::vec3 eulerAngles = glm::eulerAngles(rotQuat);
                    transform.SetRotation(eulerAngles);
                }
            }
            go.AddComponent<TransformComponent>()->transform() = transform;

            gameObjectsOnScene.push_back(go);
        }
    }
    inFile.close();

}