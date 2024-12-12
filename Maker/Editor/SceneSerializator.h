#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Engine/GameObject.h"

class SceneManager
{
public:
	static void LoadGameObject(const std::string& filePath);
	static void LoadCustomModel(const std::string& filePath);

	static void spawnBakerHouse();
	static GameObject* getGameObject(int index);
	static void addGameObject(const GameObject& gameObject);
	static void clearScene();
	//File drop handler
	static void deleteSelectedObject(); // Nueva función para eliminar el objeto seleccionado
	static void spawnParentedObjects(); // Nueva función para generar objetos parenteados

	static std::string getFileDirectory(const std::string& filePath);
	static std::string getFileNameWithoutExtension(const std::string& filePath);
	static void saveGameObject(std::ofstream& outFile, GameObject& go);
	static void saveScene(const std::string& filePath);
	static void loadScene(const std::string& filePath);


public:
	static std::vector<GameObject> gameObjectsOnScene;
	static GameObject* selectedObject;
};