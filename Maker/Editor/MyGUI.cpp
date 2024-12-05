#include "MyGUI.h"
#include "Engine/FileManager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <tinyfiledialogs/tinyfiledialogs.h> 
#include <filesystem>
#include "Engine/Log.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

inline float sanitizeZero(float value, float epsilon = 1e-6f) {
    return (std::fabs(value) < epsilon) ? 0.0f : value;
}

// Helper function to format float values
std::string formatFloat(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << value;
    std::string str = oss.str();

    // Remove trailing zeros and the decimal point if not needed
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (str.back() == '.') {
        str.pop_back();
    }

    return str;
}

MyGUI::MyGUI(SDL_Window* window, void* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();
}

MyGUI::~MyGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

FileManager fileManager;
GameObject* selectedGameObject = nullptr; // Define selectedGameObject


void MyGUI::renderConsoleWindow() {
    ImGui::SetNextWindowSize(ImVec2(480, 200), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(300, 450), ImGuiCond_Appearing);
    if (ImGui::Begin("Console", NULL)) {
        for (const auto& message : Log::getInstance().logMessages) {
            ImGui::TextUnformatted(message.c_str());
        }
    }
    ImGui::End();
}

void MyGUI::renderConfigurationWindow() {
    ImGui::SetNextWindowSize(ImVec2(480, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(300, 20), ImGuiCond_Appearing);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
    if (ImGui::Begin("Configuration", NULL)) {

        // FPS Graph
        static float fpsValues[90] = { 0 };
        static int fpsValuesOffset = 0;
        float fps = ImGui::GetIO().Framerate;
        fpsValues[fpsValuesOffset] = fps;
        fpsValuesOffset = (fpsValuesOffset + 1) % IM_ARRAYSIZE(fpsValues);
        ImGui::PlotLines("FPS", fpsValues, IM_ARRAYSIZE(fpsValues), fpsValuesOffset, "Frames Per Second", 0.0f, 120.0f, ImVec2(0, 80));

        // Configuration for modules
        if (ImGui::CollapsingHeader("Renderer")) {
            // Add renderer configuration options here
        }
        if (ImGui::CollapsingHeader("Window")) {
            // Add window configuration options here
        }
        if (ImGui::CollapsingHeader("Input")) {
            // Add input configuration options here
        }
        if (ImGui::CollapsingHeader("Textures")) {
            // Add texture configuration options here
        }

        // Information output
        if (ImGui::CollapsingHeader("Information")) {
            // Memory consumption
            ImGui::Text("%s", memoryUsage.c_str());
            // Hardware detection
            ImGui::Text("CPU Cores: %d", SDL_GetCPUCount());
            ImGui::Text("RAM: %d MB", SDL_GetSystemRAM());
            ImGui::Text("GPU: %s", glGetString(GL_RENDERER));

            // Software versions
            ImGui::Text("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
            ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
            int major = IL_VERSION / 100;
            int minor = (IL_VERSION / 10) % 10;
            int patch = IL_VERSION % 10;
            ImGui::Text("DevIL Version: %d.%d.%d", major, minor, patch);        }
    }
    ImGui::End();
}

void MyGUI::renderGameObjectWindow()
{
    // Set the size and position of the GameObject window
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);

    // Begin the GameObject window with specific flags
    if (ImGui::Begin("Gameobjects", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        // Iterate through all GameObjects in the scene and render each one
        for (auto& gameObject : scene.getChildren()) {
            renderGameObjectNode(&gameObject);
        }
    }
    // End the GameObject window
    ImGui::End();
}



void MyGUI::renderGameObjectNode(GameObject* gameObject)
{
    // Set the flags for the tree node
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (selectedGameObject == gameObject) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // Create a tree node for the GameObject
    bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), nodeFlags);
    if (ImGui::IsItemClicked()) {
        selectedGameObject = gameObject;
    }

    // Static variables for renaming functionality
    static char newName[128] = "";
    static bool renaming = false;
    static GameObject* renamingObject = nullptr;

    // Handle renaming of the GameObject
    if (renaming && renamingObject == gameObject) {
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##rename", newName, IM_ARRAYSIZE(newName), ImGuiInputTextFlags_EnterReturnsTrue)) {
            gameObject->SetName(newName);
            renaming = false;
        }
        if (ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            renaming = false;
        }
    }
    else {
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            renaming = true;
            renamingObject = gameObject;
            strcpy_s(newName, gameObject->name.c_str());
        }
    }

    // If the node is open, render its children
    if (nodeOpen) {
        for (auto& child : gameObject->children()) {
            renderGameObjectNode(&child);
        }
        ImGui::TreePop();
    }
}



void MyGUI::renderInspectorWindow()
{
    // Set the size and position of the inspector window
    ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(780, 20), ImGuiCond_Always);

    // Begin the inspector window with specific flags
    if (ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        // Check if a GameObject is selected
        if (selectedGameObject) {
            // Display the name of the selected GameObject
            ImGui::Text("Selected GameObject: %s", selectedGameObject->GetName().c_str());
            ImGui::Separator();

            // Display position
            ImGui::Text("Position");
            // Get and sanitize the position values
            float position[3] = { sanitizeZero(static_cast<float>(selectedGameObject->GetComponent<TransformComponent>()->transform().pos().x)),
                                  sanitizeZero(static_cast<float>(selectedGameObject->GetComponent<TransformComponent>()->transform().pos().y)),
                                  sanitizeZero(static_cast<float>(selectedGameObject->GetComponent<TransformComponent>()->transform().pos().z)) };

            // Format the position values as strings
            char posStr[3][32];
            snprintf(posStr[0], sizeof(posStr[0]), "%s", formatFloat(position[0]).c_str());
            snprintf(posStr[1], sizeof(posStr[1]), "%s", formatFloat(position[1]).c_str());
            snprintf(posStr[2], sizeof(posStr[2]), "%s", formatFloat(position[2]).c_str());

            // Display and edit the X position
            ImGui::Text("X:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##positionX", posStr[0], sizeof(posStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                position[0] = std::stof(posStr[0]);
            }
            // Display and edit the Y position
            ImGui::SameLine();
            ImGui::Text("Y:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##positionY", posStr[1], sizeof(posStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                position[1] = std::stof(posStr[1]);
            }
            // Display and edit the Z position
            ImGui::SameLine();
            ImGui::Text("Z:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##positionZ", posStr[2], sizeof(posStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                position[2] = std::stof(posStr[2]);
            }
            // Set the new position for the GameObject
            selectedGameObject->GetComponent<TransformComponent>()->transform().SetPosition(glm::dvec3(position[0], position[1], position[2]));

            // Display rotation
            ImGui::Text("Rotation");
            // Get and sanitize the rotation values
            glm::vec3 rotation = selectedGameObject->GetComponent<TransformComponent>()->transform().GetRotation();
            float rotationArray[3] = { sanitizeZero(rotation.x), sanitizeZero(rotation.y), sanitizeZero(rotation.z) };

            // Format the rotation values as strings
            char rotStr[3][32];
            snprintf(rotStr[0], sizeof(rotStr[0]), "%s", formatFloat(rotationArray[0]).c_str());
            snprintf(rotStr[1], sizeof(rotStr[1]), "%s", formatFloat(rotationArray[1]).c_str());
            snprintf(rotStr[2], sizeof(rotStr[2]), "%s", formatFloat(rotationArray[2]).c_str());

            // Display and edit the X rotation
            ImGui::Text("X:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##rotationX", rotStr[0], sizeof(rotStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                rotationArray[0] = std::stof(rotStr[0]);
            }
            // Display and edit the Y rotation
            ImGui::SameLine();
            ImGui::Text("Y:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##rotationY", rotStr[1], sizeof(rotStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                rotationArray[1] = std::stof(rotStr[1]);
            }
            // Display and edit the Z rotation
            ImGui::SameLine();
            ImGui::Text("Z:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##rotationZ", rotStr[2], sizeof(rotStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                rotationArray[2] = std::stof(rotStr[2]);
            }
            // Set the new rotation for the GameObject
            selectedGameObject->GetComponent<TransformComponent>()->transform().SetRotation(glm::vec3(rotationArray[0], rotationArray[1], rotationArray[2]));

            // Display scale
            ImGui::Text("Scale");
            // Get and sanitize the scale values
            glm::vec3 scale = selectedGameObject->GetComponent<TransformComponent>()->transform().GetScale();
            float scaleArray[3] = { sanitizeZero(scale.x), sanitizeZero(scale.y), sanitizeZero(scale.z) };

            // Format the scale values as strings
            char scaleStr[3][32];
            snprintf(scaleStr[0], sizeof(scaleStr[0]), "%s", formatFloat(scaleArray[0]).c_str());
            snprintf(scaleStr[1], sizeof(scaleStr[1]), "%s", formatFloat(scaleArray[1]).c_str());
            snprintf(scaleStr[2], sizeof(scaleStr[2]), "%s", formatFloat(scaleArray[2]).c_str());

            // Display and edit the X scale
            ImGui::Text("X:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##scaleX", scaleStr[0], sizeof(scaleStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                scaleArray[0] = std::stof(scaleStr[0]);
            }
            // Display and edit the Y scale
            ImGui::SameLine();
            ImGui::Text("Y:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##scaleY", scaleStr[1], sizeof(scaleStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                scaleArray[1] = std::stof(scaleStr[1]);
            }
            // Display and edit the Z scale
            ImGui::SameLine();
            ImGui::Text("Z:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText("##scaleZ", scaleStr[2], sizeof(scaleStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                scaleArray[2] = std::stof(scaleStr[2]);
            }
            // Set the new scale for the GameObject
            selectedGameObject->GetComponent<TransformComponent>()->transform().SetScale(glm::vec3(scaleArray[0], scaleArray[1], scaleArray[2]));

            ImGui::Separator();

            // Display Texture Info if the GameObject has a MeshLoader component
            if (selectedGameObject->HasComponent<MeshLoader>()) {
                if (selectedGameObject->GetComponent<MeshLoader>()->GetImage() != nullptr) {
                    ImGui::Text("Texture Info");
                    ImGui::Text("Texture Path: %s", selectedGameObject->texturePath.c_str());
                    ImGui::Text("Texture size: %d x %d", selectedGameObject->GetComponent<MeshLoader>()->GetImage()->width(), selectedGameObject->GetComponent<MeshLoader>()->GetImage()->height());
                }
                else {
                    ImGui::Text("No Texture loaded.");
                }

                // Add Texture button
                if (ImGui::Button("Texture")) {
                    const char* filterPatterns[1] = { "*.png" };
                    const char* filePath = tinyfd_openFileDialog(
                        "Select a texture file",
                        "Assets",
                        1,
                        filterPatterns,
                        NULL,
                        0
                    );
                    if (filePath) {
                        Log::getInstance().logMessage("Didac mete tu la funcion que yo no se como se hace");
                    }
                }

                // Checkbox to toggle drawing the texture
                if (ImGui::Checkbox("Draw Texture", &selectedGameObject->GetComponent<MeshLoader>()->drawTexture)) {
                    if (selectedGameObject->GetComponent<MeshLoader>()->drawTexture) {
                        selectedGameObject->GetComponent<MeshLoader>()->GetMesh()->deleteCheckerTexture();
                    }
                }

                ImGui::Separator();

                // Display Mesh Info
                ImGui::Text("Mesh Info");
                ImGui::Text("Mesh Path: %s", selectedGameObject->meshPath.c_str());
                // Checkbox to toggle drawing normals
                if (ImGui::Checkbox("Draw Normals", &selectedGameObject->GetComponent<MeshLoader>()->drawNormals)) {
                    // Handle draw normals checkbox
                }
            }
        }
        else {
            // Display message if no GameObject is selected
            ImGui::Text("No GameObject selected.");
        }
    }
    // End the inspector window
    ImGui::End();
}


void MyGUI::renderMainMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Import FBX")) {
                // Open file dialog to select an FBX file
                const char* filterPatterns[1] = { "*.fbx" };
                const char* filePath = tinyfd_openFileDialog(
                    "Select an FBX file",
                    "Assets",
                    1,
                    filterPatterns,
                    NULL,
                    0
                );
                if (filePath) {
                    std::filesystem::path path(filePath);
                    std::string fileName = path.stem().string(); // Extract the file name without extension
                    GameObject go;
                    go.SetName(fileName); // Use the extracted file name
                    fileManager.LoadFile(filePath, go);
                    scene.emplaceChild(go);
                    // This is for testing parenting, comment the lines below if you don't want to test parenting
                    // Get the first child of the scene
                    if (!scene.getChildren().empty()) {
                        GameObject& firstChild = scene.getChildren().front();
                        GameObject lastChild = scene.getChildren().back();
                        lastChild.setParent(firstChild);
                    }
                    else {
                        // If there are no children, add the new GameObject to the scene
                        scene.emplaceChild(go);
                    }
                    //
                }
            }
            if (ImGui::MenuItem("Quit")) {
                SDL_Quit();
                exit(0);
            }
            ImGui::EndMenu(); // Close the "File" menu

        }
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                // Create a new GameObject with a cube mesh
                GameObject go;
                go = scene.createCube();
                scene.emplaceChild(go);
            }
            if (ImGui::MenuItem("Sphere"))
            {
                // Create a new GameObject with a sphere mesh
                GameObject go;
                go = scene.createSphere();
                scene.emplaceChild(go);
            }
            if (ImGui::MenuItem("Cylinder"))
            {
                // Create a new GameObject with a cylinder mesh
                GameObject go;
                go = scene.createCylinder();
                scene.emplaceChild(go);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // Open a webpage when "About" is clicked
                const char* url = "https://github.com/CITM-UPC/Badass-Engine";
                SDL_OpenURL(url);
            }

            ImGui::EndMenu(); // Close the "Help" menu
        }
        ImGui::EndMainMenuBar(); // Close the main menu bar
    }

}


void MyGUI::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    //Main Menu bar, Comment the line below if you don't want the main menu bar to appear
	renderMainMenuBar();
    //GameObject window, Comment the line below if you don't want the GameObject window to appear
	renderGameObjectWindow();
    //Inspector window, Comment the line below if you don't want the Inspector window to appear
	renderInspectorWindow();
	//Console window, Comment the line below if you don't want the Console window to appear
	renderConsoleWindow();
    
    // Ensure no two GameObjects have the same name
    std::unordered_set<std::string> nameSet;
    for (auto& child : scene.getChildren()) {
        if (nameSet.find(child.GetName()) != nameSet.end()) {
            // If a duplicate name is found, append a unique suffix
            int suffix = 1;
            std::string newName = child.GetName() + "_" + std::to_string(suffix);
            while (nameSet.find(newName) != nameSet.end()) {
                suffix++;
                newName = child.GetName() + "_" + std::to_string(suffix);
            }
            child.SetName(newName);
        }
        nameSet.insert(child.GetName());
    }
    //ImGui::ShowDemoWindow();
	//render configuration window
	renderConfigurationWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MyGUI::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}
