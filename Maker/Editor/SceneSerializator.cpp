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
GameObject* SceneManager::selectedObject = nullptr;


bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

void SceneManager::addGameObject(const GameObject& gameObject) {
    GameObject go = gameObject;
    /*go.SetName("GameObject (" + std::to_string(gameObjectsOnScene.size()) + ")")*/;
    gameObjectsOnScene.push_back(go);
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
        TextureImporter::saveAsCustomImage(texture, customImagePath);
    }

    go.SetName("GameObject (" + std::to_string(gameObjectsOnScene.size()) + ")");
    gameObjectsOnScene.push_back(go);

    // Save the model in custom format
    std::string customFilePath = filePath.substr(0, filePath.find_last_of('.')) + ".custom";
    MeshImporter::saveAsCustomFormat(go, customFilePath);

    
}


GameObject* SceneManager::getGameObject(int index) {
    return &gameObjectsOnScene[index];
}

void SceneManager::deleteSelectedObject() {
    if (selectedObject) {
        auto it = std::find_if(gameObjectsOnScene.begin(), gameObjectsOnScene.end(),
            [](const GameObject& obj) { return &obj == selectedObject; });
        if (it != gameObjectsOnScene.end()) {
            gameObjectsOnScene.erase(it);
            selectedObject = nullptr;
            
        }
    }
}

void SceneManager::LoadCustomModel(const std::string& filePath) {
    try {
        GameObject go = MeshImporter::loadCustomFormat(filePath);
        go.SetName("Custom Model (" + std::to_string(gameObjectsOnScene.size()) + ")");
        gameObjectsOnScene.push_back(go);
        
    }
    catch (const std::exception& e) {
        
    }
}

void SceneManager::saveGameObject(std::ofstream& outFile, GameObject& go) {
    const auto& transform = go.GetComponent<TransformComponent>()->transform();

    outFile << "  {\n";
    outFile << "    \"Id\": " << go.getId() << ",\n";
    outFile << "    \"Name\": \"" << go.GetName() << "\",\n";
    outFile << "    \"Transform\": {\n";
    outFile << "      \"Position\": [" << transform.pos().x << ", " << transform.pos().y << ", " << transform.pos().z << "],\n";
    outFile << "      \"Scale\": [" << transform.GetScale().x << ", "
        << transform.GetScale().y << ", "
        << transform.GetScale().z << "],\n";
    outFile << "      \"Rotation\": [" << transform.GetRotation().x << ", "
        << transform.GetRotation().y << ", "
        << transform.GetRotation().z << "]\n";
    outFile << "    },\n";

    if (go.hasMesh()) {
        outFile << "    \"Mesh\": \"BinaryDataStart\"\n"; // Indicate binary section
        const auto& mesh = go.mesh();

        // Write binary mesh data
        uint32_t vertexCount = mesh.vertices().size();
        uint32_t indexCount = mesh.indices().size();

        outFile.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        outFile.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
        outFile.write(reinterpret_cast<const char*>(mesh.vertices().data()), vertexCount * sizeof(glm::vec3));
        outFile.write(reinterpret_cast<const char*>(mesh.indices().data()), indexCount * sizeof(unsigned int));
    }
    else {
        outFile << "    \"Mesh\": null\n";
    }

    outFile << "  }";
}

void SceneManager::saveScene(const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: No se pudo abrir el archivo para escribir." << std::endl;
        return;
    }
    
    // Asegurarse de que todos los GameObjects están en la lista
    for (const auto& go : scene.children()) {
        addGameObject(go);
    }

    // Write scene metadata (start as text or binary marker)
    outFile << "{\n\"GameObjects\": [\n";

    for (size_t i = 0; i < gameObjectsOnScene.size(); ++i) {
        saveGameObject(outFile, gameObjectsOnScene[i]);
        if (i != gameObjectsOnScene.size() - 1) outFile << ",";
        outFile << "\n";
    }

    outFile << "]\n}";
    outFile.close();

    if (!outFile) {
        std::cerr << "Error: No se pudo abrir el archivo para escribir." << std::endl;
    }
}

void SceneManager::clearScene() {
    gameObjectsOnScene.clear();
	for (GameObject& go : scene.getChildren()) {
		go.DeleteGameObject();
	}
    selectedObject = nullptr;
}

void SceneManager::loadScene(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: No se pudo leer correctamente en el archivo." << std::endl;
        return;
    }

    clearScene();

    //std::string line;
    //while (std::getline(inFile, line)) {
    //    if (line.find("\"Id\":") != std::string::npos) {
    //        GameObject go;
    //        go.setId(std::stoi(line.substr(line.find(":") + 1)));

    //        // Parse Name
    //        std::getline(inFile, line);
    //        if (line.find("\"Name\":") != std::string::npos) {
    //            std::string name = line.substr(line.find(":") + 2);
    //            name.erase(name.find_last_of("\""));
    //            go.SetName(name);
    //        }

    //        // Parse Transform
    //        Transform transform;
    //        while (std::getline(inFile, line) && line.find("}") == std::string::npos) {
    //            if (line.find("\"Position\":") != std::string::npos) {
    //                std::istringstream iss(line.substr(line.find("[") + 1));
    //                glm::vec3 pos;
    //                char delim;
    //                iss >> pos.x >> delim >> pos.y >> delim >> pos.z;
    //                transform.SetPosition(pos);
    //            }
    //            else if (line.find("\"Scale\":") != std::string::npos) {
    //                std::istringstream iss(line.substr(line.find("[") + 1));
    //                glm::vec3 scale;
    //                char delim;
    //                iss >> scale.x >> delim >> scale.y >> delim >> scale.z;
    //                transform.SetScale(scale);
    //            }
    //            else if (line.find("\"Rotation\":") != std::string::npos) {
    //                std::istringstream iss(line.substr(line.find("[") + 1));
    //                glm::vec3 rotation;
    //                char delim;
    //                iss >> rotation.x >> delim >> rotation.y >> delim >> rotation.z;
    //                transform.SetRotation(rotation);
    //            }
    //        }
    //        go.AddComponent<TransformComponent>()->transform() = transform;

    //        // Parse Mesh
    //        std::getline(inFile, line);
    //        if (line.find("\"Mesh\": \"BinaryDataStart\"") != std::string::npos) {
    //            auto mesh = std::make_shared<Mesh>();

    //            uint32_t vertexCount, indexCount;
    //            inFile.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
    //            inFile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));

    //            std::vector<glm::vec3> vertices(vertexCount);
    //            std::vector<unsigned int> indices(indexCount);

    //            inFile.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(glm::vec3));
    //            inFile.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(unsigned int));

    //            mesh->load(vertices.data(), vertices.size(), indices.data(), indices.size());
    //            go.setMesh(mesh);
    //        }

    //        gameObjectsOnScene.push_back(go);
    //    }
    //}

    inFile.close();
    
}