#include "MyGUI.h"
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
#include "Engine/GameObject.h"

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

static GameObject* persistentSelectedGameObject = nullptr; 
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

void MyGUI::renderGameObjectWindow() {
    // Set the size and position of the GameObject window
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);

    // Begin the GameObject window with specific flags
    if (ImGui::Begin("Gameobjects", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        // Check if right-clicked on the window
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("WindowContextMenu");
        }

        if (ImGui::BeginPopup("WindowContextMenu")) {
            if (ImGui::MenuItem("Create Empty")) {
                pendingOperations.push([]() {
                    GameObject go = scene.CreateEmpty();
                    scene.emplaceChild(go);
                    });
            }
            if (ImGui::MenuItem("Create Cube")) {
                pendingOperations.push([]() {
                    GameObject go = scene.createCube();
                    scene.emplaceChild(go);
                    });
            }
            if (ImGui::MenuItem("Create Sphere")) {
                pendingOperations.push([]() {
                    GameObject go = scene.createSphere();
                    scene.emplaceChild(go);
                    });
            }
            if (ImGui::MenuItem("Create Cylinder")) {
                pendingOperations.push([]() {
                    GameObject go = scene.createCylinder();
                    scene.emplaceChild(go);
                    });
            }
            ImGui::EndPopup();
        }

        // Iterate through all GameObjects in the scene and render each one
        for (auto& gameObject : scene.getChildren()) {
            renderGameObjectNode(&gameObject);
        }
    }
    // End the GameObject window
    ImGui::End();

    // Execute pending operations
    while (!pendingOperations.empty()) {
        pendingOperations.front()();
        pendingOperations.pop();
    }
}

void MyGUI::renderGameObjectNode(GameObject* gameObject)
{
    // Set the flags for the tree node
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (persistentSelectedGameObject == gameObject) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // Check if the GameObject has children
    if (gameObject->getChildren().empty()) {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // Create a tree node for the GameObject
    bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), nodeFlags);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        persistentSelectedGameObject = gameObject;
        isSelectedFromWindow = true; // Set the flag when selected from the window
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
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            renaming = true;
            renamingObject = gameObject;
            strcpy_s(newName, gameObject->GetName().c_str());
        }
    }

    // Check if right-clicked on the GameObject
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(("GameObjectContextMenu" + std::to_string(reinterpret_cast<uintptr_t>(gameObject))).c_str());
    }

    if (ImGui::BeginPopup(("GameObjectContextMenu" + std::to_string(reinterpret_cast<uintptr_t>(gameObject))).c_str())) {
        if (ImGui::MenuItem("Delete")) {
            Log::getInstance().logMessage("Deleted GameObject: " + gameObject->GetName() + "(Just kidding, the function isn't implemented yet)");
            // Add functionality here
        }
        if (ImGui::MenuItem("Rename")) {
            renaming = true;
            renamingObject = gameObject;
            strcpy_s(newName, gameObject->GetName().c_str());
        }
        if (ImGui::MenuItem("Create Empty Child")) {
            pendingOperations.push([gameObject]() {
                GameObject go = scene.CreateEmptyChild(*gameObject);;
                });
        }
        ImGui::EndPopup();
    }
    // Handle drag source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("DND_GAMEOBJECT", &gameObject, sizeof(GameObject*));
        ImGui::Text("Reparent %s", gameObject->GetName().c_str());
        ImGui::EndDragDropSource();
    }

    // Handle drag target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT")) {
            GameObject* droppedGameObject = *(GameObject**)payload->Data;
            if (droppedGameObject != gameObject) {
				pendingOperations.push([gameObject, droppedGameObject]() {
                    GameObject go = *droppedGameObject;
					go.setParent(*gameObject);
					selectedGameObject = gameObject;
					persistentSelectedGameObject = gameObject;
					});
            }
        }
        ImGui::EndDragDropTarget();
    }
    // Recursively render children if the node is open
    if (nodeOpen && !gameObject->getChildren().empty()) {
        for (auto& child : gameObject->getChildren()) {
            renderGameObjectNode(&child);
        }
        ImGui::TreePop();
    }
}


 void renderAssetNode(const std::filesystem::path& path, std::filesystem::path& selectedPath) {
    // Configura los flags del nodo del árbol
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (std::filesystem::is_directory(path)) {
        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    }
    else {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // Crea un nodo del árbol para el asset
    bool nodeOpen = ImGui::TreeNodeEx(path.filename().string().c_str(), nodeFlags);
    if (ImGui::IsItemClicked()) {
        selectedPath = path;
    }

    // Si el nodo está abierto y es un directorio, renderiza sus hijos
    if (nodeOpen && std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            renderAssetNode(entry.path(), selectedPath);
        }
        ImGui::TreePop();
    }
}

void MyGUI::renderAssetWindow() {
    // Set the size and position of the asset window
    ImGui::SetNextWindowSize(ImVec2(480, 200), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_Appearing); // Adjusted Y-coordinate to move the window up

    // Begin the asset window with specific flags
    if (ImGui::Begin("Assets", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        // Asset directory path
        std::filesystem::path assetDirectory = "Assets";
        static std::filesystem::path selectedPath;

        // Render the root node of the asset tree
        renderAssetNode(assetDirectory, selectedPath);

        // Detect if the "Delete" key is pressed and a file or directory is selected
        if (!selectedPath.empty() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
            try {
                if (std::filesystem::is_directory(selectedPath)) {
                    std::filesystem::remove_all(selectedPath);
                }
                else {
                    std::filesystem::remove(selectedPath);
                }
                selectedPath.clear(); // Clear the selection after deletion
            }
            catch (const std::filesystem::filesystem_error& e) {
                // Handle the error if it occurs
                std::cerr << "Error deleting the file or directory: " << e.what() << std::endl;
            }
        }
    }
    // End the asset window
    ImGui::End();
}



void MyGUI::ManagePosition()
{
    ImGui::Text("Position");
    // Get and sanitize the position values
    float position[3] = { sanitizeZero(static_cast<float>(persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().pos().x)),
                          sanitizeZero(static_cast<float>(persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().pos().y)),
                          sanitizeZero(static_cast<float>(persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().pos().z)) };

    // Format the position values as strings
    char posStr[3][32];
    snprintf(posStr[0], sizeof(posStr[0]), "%s", formatFloat(position[0]).c_str());
    snprintf(posStr[1], sizeof(posStr[1]), "%s", formatFloat(position[1]).c_str());
    snprintf(posStr[2], sizeof(posStr[2]), "%s", formatFloat(position[2]).c_str());

    bool positionChanged = false;

    // Display and edit the X position
    ImGui::Text("X:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##positionX", posStr[0], sizeof(posStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        position[0] = std::stof(posStr[0]);
        positionChanged = true;
    }
    // Display and edit the Y position
    ImGui::SameLine();
    ImGui::Text("Y:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##positionY", posStr[1], sizeof(posStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        position[1] = std::stof(posStr[1]);
        positionChanged = true;
    }
    // Display and edit the Z position
    ImGui::SameLine();
    ImGui::Text("Z:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##positionZ", posStr[2], sizeof(posStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        position[2] = std::stof(posStr[2]);
        positionChanged = true;
    }

    // Set the new position for the GameObject only if a change was made
    if (positionChanged) {
        persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().SetPosition(glm::dvec3(position[0], position[1], position[2]));
    }
    
}

void MyGUI::ManageRotation()
{
    ImGui::Text("Rotation");
    // Get and sanitize the rotation values
    glm::vec3 rotation = persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().GetRotation();
    float rotationArray[3] = { sanitizeZero(rotation.x), sanitizeZero(rotation.y), sanitizeZero(rotation.z) };

    // Format the rotation values as strings
    char rotStr[3][32];
    snprintf(rotStr[0], sizeof(rotStr[0]), "%s", formatFloat(rotationArray[0]).c_str());
    snprintf(rotStr[1], sizeof(rotStr[1]), "%s", formatFloat(rotationArray[1]).c_str());
    snprintf(rotStr[2], sizeof(rotStr[2]), "%s", formatFloat(rotationArray[2]).c_str());

    bool rotationChanged = false;

    // Display and edit the X rotation
    ImGui::Text("X:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##rotationX", rotStr[0], sizeof(rotStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        rotationArray[0] = std::stof(rotStr[0]);
        rotationChanged = true;
    }
    // Display and edit the Y rotation
    ImGui::SameLine();
    ImGui::Text("Y:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##rotationY", rotStr[1], sizeof(rotStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        rotationArray[1] = std::stof(rotStr[1]);
        rotationChanged = true;
    }
    // Display and edit the Z rotation
    ImGui::SameLine();
    ImGui::Text("Z:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##rotationZ", rotStr[2], sizeof(rotStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        rotationArray[2] = std::stof(rotStr[2]);
        rotationChanged = true;
    }

    // Set the new rotation for the GameObject only if a change was made
    if (rotationChanged) {
        persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().SetRotation(glm::vec3(rotationArray[0], rotationArray[1], rotationArray[2]));
    }
   
}


void MyGUI::ManageScale()
{
    ImGui::Text("Scale");
    // Get and sanitize the scale values
    glm::vec3 scale = persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().GetScale();
    float scaleArray[3] = { sanitizeZero(scale.x), sanitizeZero(scale.y), sanitizeZero(scale.z) };

    // Format the scale values as strings
    char scaleStr[3][32];
    snprintf(scaleStr[0], sizeof(scaleStr[0]), "%s", formatFloat(scaleArray[0]).c_str());
    snprintf(scaleStr[1], sizeof(scaleStr[1]), "%s", formatFloat(scaleArray[1]).c_str());
    snprintf(scaleStr[2], sizeof(scaleStr[2]), "%s", formatFloat(scaleArray[2]).c_str());

    bool scaleChanged = false;

    // Display and edit the X scale
    ImGui::Text("X:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##scaleX", scaleStr[0], sizeof(scaleStr[0]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        scaleArray[0] = std::stof(scaleStr[0]);
        scaleChanged = true;
    }
    // Display and edit the Y scale
    ImGui::SameLine();
    ImGui::Text("Y:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##scaleY", scaleStr[1], sizeof(scaleStr[1]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        scaleArray[1] = std::stof(scaleStr[1]);
        scaleChanged = true;
    }
    // Display and edit the Z scale
    ImGui::SameLine();
    ImGui::Text("Z:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##scaleZ", scaleStr[2], sizeof(scaleStr[2]), ImGuiInputTextFlags_EnterReturnsTrue)) {
        scaleArray[2] = std::stof(scaleStr[2]);
        scaleChanged = true;
    }

    // Set the new scale for the GameObject only if a change was made
    if (scaleChanged) {
        persistentSelectedGameObject->GetComponent<TransformComponent>()->transform().SetScale(glm::vec3(scaleArray[0], scaleArray[1], scaleArray[2]));
    }
    
}

void MyGUI::renderInspectorWindow()
{
    if (selectedGameObject != nullptr) {
        persistentSelectedGameObject = selectedGameObject;
    }

    // Set the size and position of the inspector window
    ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(780, 20), ImGuiCond_Always);

    // Begin the inspector window with specific flags
    if (ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        // Check if a GameObject is selected
        if (persistentSelectedGameObject) {
            // Display the name of the selected GameObject
            ImGui::Text("Selected GameObject: %s", persistentSelectedGameObject->GetName().c_str());
            ImGui::Separator();

            ManagePosition();
            ManageRotation();
            ManageScale();

            ImGui::Separator();

            // Display Texture Info if the GameObject has a MeshLoader component
            if (persistentSelectedGameObject->HasComponent<MeshLoader>()) {
                if (persistentSelectedGameObject->GetComponent<MeshLoader>()->GetImage() != nullptr) {
                    ImGui::Text("Texture Info");
                    ImGui::Text("Texture Path: %s", persistentSelectedGameObject->texturePath.c_str());
                    ImGui::Text("Texture size: %d x %d", persistentSelectedGameObject->GetComponent<MeshLoader>()->GetImage()->width(), persistentSelectedGameObject->GetComponent<MeshLoader>()->GetImage()->height());
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
                        fileManager.LoadTexture(filePath, *persistentSelectedGameObject);
                        Log::getInstance().logMessage("Loaded Texture from:");
                        Log::getInstance().logMessage(filePath);
                    }
                }

                // Checkbox to toggle drawing the texture
                if (ImGui::Checkbox("Draw Texture", &persistentSelectedGameObject->GetComponent<MeshLoader>()->drawTexture)) {
                    if (persistentSelectedGameObject->GetComponent<MeshLoader>()->drawTexture) {
                        persistentSelectedGameObject->GetComponent<MeshLoader>()->GetMesh()->deleteCheckerTexture();
                    }
                }

                ImGui::Separator();

                // Display Mesh Info
                ImGui::Text("Mesh Info");
                ImGui::Text("Mesh Path: %s", persistentSelectedGameObject->meshPath.c_str());
                // Checkbox to toggle drawing normals
                if (ImGui::Checkbox("Draw Normals", &persistentSelectedGameObject->GetComponent<MeshLoader>()->drawNormals)) {
                    // Handle draw normals checkbox
                }
            }

            ImGui::Separator();

            // Display Camera Component if the GameObject has a Camera component
            if (persistentSelectedGameObject->HasComponent<CameraComponent>()) {
                // Display Camera Info
            }

            ImGui::Separator();

            // Display the name of the parent GameObject
            GameObject* parent = nullptr;
            parent = &persistentSelectedGameObject->parent();
            if (parent) {
                ImGui::Text("Parent GameObject: %s", parent->GetName().c_str());
            }
            else {
                ImGui::Text("Parent GameObject: None");
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

                    
                   
                }
            }
            if (ImGui::MenuItem("Quit")) {
                SDL_Quit();
                exit(0);
            }
            if (ImGui::MenuItem("Load Custom"))
            {
                // Testing Load Custom Format File
				GameObject go;
				go.SetName("Custom");
				fileManager.LoadCustomFile("Library/Meshes/mesh.mesh", go);
				scene.emplaceChild(go);
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

void MyGUI::CheckForDuplicateNames()
{
    for (auto& child : scene.getChildren()) {
        CheckForDuplicateNamesRecursive(child);
    }
}

void MyGUI::CheckForDuplicateNamesRecursive(GameObject& gameObject)
{
    std::unordered_set<std::string> nameSet;
    int maxSuffix = 0;

    // Collect names of siblings
    GameObject* parent = nullptr;
    if (parent = &gameObject.parent()) {
        for (auto& sibling : parent->getChildren()) {
            if (&sibling != &gameObject) {
                nameSet.insert(sibling.GetName());
            }
        }
    }
    else {
        for (auto& sibling : scene.getChildren()) {
            if (&sibling != &gameObject) {
                nameSet.insert(sibling.GetName());
            }
        }
    }

    std::string originalName = gameObject.GetName();
    std::string newName = originalName;

    // Check if the name already exists in the set
    if (nameSet.find(newName) != nameSet.end()) {
        // Find the highest suffix used
        int suffix = 1;
        while (nameSet.find(newName) != nameSet.end()) {
            newName = originalName + "(" + std::to_string(suffix) + ")";
            suffix++;
        }
        gameObject.SetName(newName);
    }

    // Recursively check for duplicates in the children
    for (auto& child : gameObject.getChildren()) {
        CheckForDuplicateNamesRecursive(child);
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
	//Asset window, Comment the line below if you don't want the Asset window to appear
	renderAssetWindow();
    
    CheckForDuplicateNames();
    //ImGui::ShowDemoWindow();
	//render configuration window
	renderConfigurationWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MyGUI::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}
