#pragma once

#include "MyWindow.h"
#include "Engine/Scene.h"
#include <list>
#include <string>
#include <vector> // Include the vector header
#include <imgui.h>

using namespace std;
extern GameObject* selectedGameObject; // Define selectedGameObject

class MyGUI
{
public:
    MyGUI(SDL_Window* window, void* context);
    MyGUI(MyGUI&&) noexcept = delete;
    MyGUI(const MyGUI&) = delete;
    MyGUI& operator=(const MyGUI&) = delete;
    ~MyGUI();
    void renderConfigurationWindow();
    void renderGameObjectWindow();
    void renderInspectorWindow();
    void renderMainMenuBar();
    void render();
    void processEvent(const SDL_Event& event);
    void renderConsoleWindow();
    void renderAssetWindow();
    void renderGameObjectNode(GameObject* gameObject);
	void renderAssetsWindow();

    void ManagePosition();

    void ManageRotation();

    void ManageScale();

    string memoryUsage;
private:

    

};

extern MyGUI* gui; // Define myGUI


