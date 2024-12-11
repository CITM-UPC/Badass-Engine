#include <yaml-cpp/yaml.h>
#include "Engine/GameObject.h"
#include <fstream>

void serializeGameObject(YAML::Emitter& out, const GameObject& gameObject) {
    out << YAML::BeginMap;
    out << YAML::Key << "id" << YAML::Value << gameObject.getId();
    out << YAML::Key << "name" << YAML::Value << gameObject.GetName();
    out << YAML::Key << "tag" << YAML::Value << gameObject.tag;
    out << YAML::Key << "active" << YAML::Value << gameObject.active;
    out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

    // Serializar componentes del GameObject
    for (const auto& [type, component] : gameObject.components) {
        out << YAML::BeginMap;
        out << YAML::Key << "type" << YAML::Value << type.name();
        // Aquí puedes agregar la serialización específica de cada componente
        out << YAML::EndMap;
    }

    out << YAML::EndSeq;

    // Serializar hijos del GameObject
    out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;
    for (const auto& child : gameObject.GetChildren()) {
        serializeGameObject(out, *child);
    }
    out << YAML::EndSeq;

    out << YAML::EndMap;
}

void saveGameObjectToFile(const GameObject& gameObject, const std::string& filePath) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "GameObject" << YAML::Value;
    serializeGameObject(out, gameObject);
    out << YAML::EndMap;

    // Guardar el YAML en un archivo
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << out.c_str();
        file.close();
    }
    else {
        // Log::getInstance().logMessages.push_back("Error al abrir el archivo para guardar el GameObject.");
    }
}

std::shared_ptr<GameObject> deserializeGameObject(const YAML::Node& node) {
    auto gameObject = std::make_shared<GameObject>(node["name"].as<std::string>());
    gameObject->id = node["id"].as<int>();
    gameObject->tag = node["tag"].as<std::string>();
    gameObject->active = node["active"].as<bool>();

    // Deserializar componentes del GameObject
    const auto& componentsNode = node["components"];
    for (const auto& componentNode : componentsNode) {
        std::string typeName = componentNode["type"].as<std::string>();
        // Aquí puedes agregar la deserialización específica de cada componente
    }

    // Deserializar hijos del GameObject
    const auto& childrenNode = node["children"];
    for (const auto& childNode : childrenNode) {
        auto child = deserializeGameObject(childNode);
        gameObject->_myChildren.push_back(child);
    }

    return gameObject;
}

std::shared_ptr<GameObject> loadGameObjectFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        /*std::cerr << "Error al abrir el archivo para cargar el GameObject." << std::endl;*/
        return nullptr;
    }

    YAML::Node root = YAML::Load(file);
    file.close();
    
    if (!root["GameObject"]) {
        /*std::cerr << "El archivo no contiene un GameObject válido." << std::endl;*/
        return nullptr;
    }

    return deserializeGameObject(root["GameObject"]);
}