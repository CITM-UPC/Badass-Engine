#pragma once

#include <yaml-cpp/yaml.h>
#include "Engine/GameObject.h"
#include <fstream>

void serializeGameObject(YAML::Emitter& out, const GameObject& gameObject);

void saveGameObjectToFile(const GameObject& gameObject, const std::string& filePath);

std::shared_ptr<GameObject> deserializeGameObject(const YAML::Node& node);

void loadGameObjectFromFile(const std::string& filePath, std::shared_ptr<GameObject> gameObject);