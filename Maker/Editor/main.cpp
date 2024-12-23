#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <string>
using namespace std;
#include "Engine/GameObject.h"
#include "Engine/Scene.h"
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Engine/Camera.h"
#include "../Engine/Mesh.h"
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <exception>
#include "MyWindow.h"
#include "MyGUI.h"
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/log.h"
#include <windows.h>
#include <psapi.h>
#include <iomanip>
#include <sstream>
#include "MeshImporter.h"

void GetMemoryUsage(MyGUI& gui) {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		double privateUsageMB = pmc.PrivateUsage / (1024.0 * 1024.0);
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << privateUsageMB;
		gui.memoryUsage = "Memory consumption: " + ss.str() + " MB";
	}
	else {
		gui.memoryUsage = "Failed to get memory usage information.";
	}
}


using hrclock = chrono::high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;

static const ivec2 WINDOW_SIZE(1280, 720);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

GameObject mainCamera("Main Camera");
glm::dmat4 projectionMatrix;
glm::dmat4 viewMatrix;
glm::dmat4 viewProjectionMatrix;

vec3 pointSelected(0.0f, 0.0f, 0.0f);
float angleX = 0.0f;
float angleY = 0.0f;
float radius = 10.0f; 
float sensibility = 0.1f;
static bool middleMousePressed = false;
static ivec2 lastMousePosition;
static bool rightMousePressed = false;
static bool leftMousePressed = false;
static vector <GameObject> gameObjects;
GameObject* selectedObject = nullptr;
static double moveSpeed = 0.1;
float yaw = 0.0f;
float pitch = 0.0f;
float initialYaw = 0.0f;
float initialPitch = 0.0f;
const float MAX_PITCH = 89.0f;
bool altKeyPressed = false;
vec3 target;
ivec2 delta;
bool isFpressed = false;
int lastMouseX;
int lastMouseY;

vec3 TurnScreenCoordinates(int mouseX, int mouseY) {

	// Obtener el tama�o de la ventana para calcular las coordenadas normalizadas
	int screenWidth = WINDOW_SIZE.x;
	int screenHeight = WINDOW_SIZE.y;

	// Normalizar las coordenadas del rat�n
	float x = (2.0f * mouseX) / screenWidth - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / screenHeight;
	float z = 1.0f;

	// Crear un vector de coordenadas normalizadas
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	// Convertir a coordenadas homog�neas
	glm::vec4 ray_clip = glm::vec4(ray_nds, 1.0);

	// Convertir a coordenadas de ojo
	glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	// Convertir a coordenadas del mundo
	glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * ray_eye));

	// Calcular el punto 3D de intersecci�n del rayo con un plano en la escena (ej., plano del suelo)
	float planeHeight = 0.0f; // Por ejemplo, un plano en Y = 0
	float t = (planeHeight - mainCamera.GetComponent<CameraComponent>()->camera().transform().pos().y) / ray_world.y;

	// Calcular la posici�n del punto en el plano de la intersecci�n
	glm::vec3 intersectionPoint = glm::vec3(mainCamera.GetComponent<CameraComponent>()->camera().transform().pos()) + t * ray_world;

	return intersectionPoint;
}
// Function to convert screen coordinates to world coordinates
glm::vec3 screenToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	// Normalize mouse coordinates to [-1, 1]
	float x = (2.0f * mouseX) / screenWidth - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / screenHeight;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	// Convert to homogeneous clip coordinates
	glm::vec4 ray_clip = glm::vec4(ray_nds, 1.0);

	// Convert to eye coordinates
	glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	// Convert to world coordinates
	glm::vec3 ray_wor = glm::vec3(glm::inverse(viewMatrix) * ray_eye);
	ray_wor = glm::normalize(ray_wor);

	return ray_wor;
}

bool rayIntersectsBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const BoundingBox& bbox) {
	float tmin = (bbox.min.x - rayOrigin.x) / rayDir.x;
	float tmax = (bbox.max.x - rayOrigin.x) / rayDir.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (bbox.min.y - rayOrigin.y) / rayDir.y;
	float tymax = (bbox.max.y - rayOrigin.y) / rayDir.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (bbox.min.z - rayOrigin.z) / rayDir.z;
	float tzmax = (bbox.max.z - rayOrigin.z) / rayDir.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	return true;
}

glm::vec3 getRayFromMouse(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize) {
	float x = (2.0f * mouseX) / viewportSize.x - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / viewportSize.y;
	glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

	glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
	return rayWorld;
}

GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize) {
	// Obtener el origen del rayo en coordenadas del mundo
	glm::vec3 rayOrigin = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));

	// Obtener la direcci�n del rayo desde las coordenadas del rat�n
	glm::vec3 rayDirection = getRayFromMouse(mouseX, mouseY, projection, view, viewportSize);

	GameObject* hitObject = nullptr;

	// Iterar sobre todos los objetos en la escena
	for (auto& go : scene.getChildren()) {
		// Verificar si el rayo intersecta con el BoundingBox del objeto
		if (rayIntersectsBoundingBox(rayOrigin, rayDirection, go.boundingBox())) {
			hitObject = &go;
			break;
		}
	}

	// Si no se encuentra ning�n objeto, hitObject ser� nullptr
	return hitObject;
}

static void drawFloorGrid(int size, double step) {
	glBegin(GL_LINES);
	for (double i = -size; i <= size; i += step) {
		glVertex3d(i, 0, -size);
		glVertex3d(i, 0, size);
		glVertex3d(-size, 0, i);
		glVertex3d(size, 0, i);
	}
	glEnd();
}

void DrawFrustum(const Frustum& frustum)
{
	glColor3f(1, 1, 1);
	glLineWidth(0.5);

	// Draw near plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(frustum.vertices[0].x, frustum.vertices[0].y, frustum.vertices[0].z);
	glVertex3f(frustum.vertices[1].x, frustum.vertices[1].y, frustum.vertices[1].z);
	glVertex3f(frustum.vertices[2].x, frustum.vertices[2].y, frustum.vertices[2].z);
	glVertex3f(frustum.vertices[3].x, frustum.vertices[3].y, frustum.vertices[3].z);
	glEnd();

	// Draw far plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(frustum.vertices[4].x, frustum.vertices[4].y, frustum.vertices[4].z);
	glVertex3f(frustum.vertices[5].x, frustum.vertices[5].y, frustum.vertices[5].z);
	glVertex3f(frustum.vertices[6].x, frustum.vertices[6].y, frustum.vertices[6].z);
	glVertex3f(frustum.vertices[7].x, frustum.vertices[7].y, frustum.vertices[7].z);
	glEnd();

	// Draw lines connecting near and far planes
	glBegin(GL_LINES);
	glVertex3f(frustum.vertices[0].x, frustum.vertices[0].y, frustum.vertices[0].z);
	glVertex3f(frustum.vertices[4].x, frustum.vertices[4].y, frustum.vertices[4].z);

	glVertex3f(frustum.vertices[1].x, frustum.vertices[1].y, frustum.vertices[1].z);
	glVertex3f(frustum.vertices[5].x, frustum.vertices[5].y, frustum.vertices[5].z);

	glVertex3f(frustum.vertices[2].x, frustum.vertices[2].y, frustum.vertices[2].z);
	glVertex3f(frustum.vertices[6].x, frustum.vertices[6].y, frustum.vertices[6].z);

	glVertex3f(frustum.vertices[3].x, frustum.vertices[3].y, frustum.vertices[3].z);
	glVertex3f(frustum.vertices[7].x, frustum.vertices[7].y, frustum.vertices[7].z);
	glEnd();
}

void configureCamera() {
	projectionMatrix = mainCamera.GetComponent<CameraComponent>()->camera().projection();
	viewMatrix = mainCamera.GetComponent<CameraComponent>()->camera().view();
	viewProjectionMatrix = mainCamera.GetComponent<CameraComponent>()->camera().viewProjection();
	mainCamera.GetComponent<CameraComponent>()->camera().UpdateMainCamera();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(glm::value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(glm::value_ptr(viewMatrix));
}

void updateGameObjectAndChildren(GameObject& gameObject) {
	// Draw the current game object

	GameObject testCamera;
	// Testing Frustum Culling
	for (auto& child : scene.children()) {
		if (child.name == "Test Camera")
		{
			testCamera = child;
		}
	}
	
	if (gameObject.HasComponent<MeshLoader>() && testCamera.GetComponent<CameraComponent>()->camera().frustum.ContainsBBox(gameObject.boundingBox()) == 1 || testCamera.GetComponent<CameraComponent>()->camera().frustum.ContainsBBox(gameObject.boundingBox()) == 2) {
		gameObject.draw();
	}
	
	if (gameObject.HasComponent<CameraComponent>() && gameObject.name != "Main Camera") {
		gameObject.GetComponent<CameraComponent>()->camera().UpdateCamera(gameObject.GetComponent<TransformComponent>()->transform());
		DrawFrustum(gameObject.GetComponent<CameraComponent>()->camera().frustum);
	}

	gameObject.drawDebug(gameObject);

	// Recursively draw all children
	for (auto& child : gameObject.children()) {
		updateGameObjectAndChildren(child);
	}
}

void updateScene() {
	// Draw all top-level children of the scene
	for (auto& child : scene.children()) {
		updateGameObjectAndChildren(child);
	}
}

static void display_func() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	configureCamera();
	drawFloorGrid(16, 0.25);

	updateScene();

	//scene.drawDebug(scene);

	cout << "Number of children: " << scene.children().size() << endl;
	
}

static void init_opengl() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void reshape_func(int width, int height) {
	glViewport(0, 0, width, height);
	mainCamera.GetComponent<CameraComponent>()->camera().aspect = static_cast<double>(width) / height;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(&mainCamera.GetComponent<CameraComponent>()->camera().projection()[0][0]);
}

static void mouseWheel_func(int direction) {
	// Mueve la c�mara hacia adelante o hacia atr�s
	mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(vec3(0, 0, direction * 0.1));
}

glm::vec2 getMousePosition() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void orbitCamera(const vec3& target, int deltaX, int deltaY) {
	const float sensitivity = 0.1f;
	const float maxPitch = 89.0f; 

	
	yaw += deltaX * sensitivity;
	pitch -= deltaY * sensitivity;

	
	if (pitch > maxPitch) pitch = maxPitch;
	if (pitch < -maxPitch) pitch = -maxPitch;

	
	float distance = glm::length(mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() - target);

	
	vec3 newPosition;
	newPosition.x = target.x + distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	newPosition.y = target.y + distance * sin(glm::radians(pitch));
	newPosition.z = target.z + distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));

	
	mainCamera.GetComponent<CameraComponent>()->camera().transform().SetPosition(newPosition);
	mainCamera.GetComponent<CameraComponent>()->camera().transform().lookAt(target);
}

static void mouseButton_func(int button, int state, int x, int y) {
	if (button == SDL_BUTTON_MIDDLE) {
		middleMousePressed = (state == SDL_PRESSED);
		if (middleMousePressed) {
			lastMousePosition = ivec2(x, y);
		}
	}
	if (button == SDL_BUTTON_RIGHT) {
		rightMousePressed = (state == SDL_PRESSED);
		// Calcular la diferencia de movimiento del rat�n
		if (rightMousePressed) {
			lastMouseX = x;
			lastMouseY = y;
			initialYaw = yaw;
			initialPitch = pitch;
		}
		
	}
	if (button == SDL_BUTTON_LEFT) {
		leftMousePressed = (state == SDL_PRESSED);
		if (leftMousePressed) {
			// Guarda la posici�n del rat�n al hacer clic
			/*lastMousePosition = ivec2(x, y);*/
			lastMouseX = x;
			lastMouseY = y;

			initialYaw = yaw;
			initialPitch = pitch;

			pointSelected = TurnScreenCoordinates(x, y);
			std::cout << "Punto seleccionado: " << pointSelected.x << ", " << pointSelected.y << ", " << pointSelected.z << std::endl;
			
		}
	}
}
static void handleKeyboardInput() {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (rightMousePressed) {

		if (state[SDL_SCANCODE_W]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, 0, moveSpeed));
		}
		if (state[SDL_SCANCODE_S]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, 0, -moveSpeed));
		}
		if (state[SDL_SCANCODE_A]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(moveSpeed, 0, 0));
		}
		if (state[SDL_SCANCODE_D]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(-moveSpeed, 0, 0));
		}
		if (state[SDL_SCANCODE_E]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, -moveSpeed, 0));
		}
		if (state[SDL_SCANCODE_Q]) {
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, moveSpeed, 0));
		}
		if (state[SDL_SCANCODE_LSHIFT]) {
			moveSpeed = 0.2;
		}
		else {
			moveSpeed = 0.1;
		}
	}

	if (state[SDL_SCANCODE_F] && selectedGameObject != nullptr) {
		isFpressed != isFpressed;
		mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() = selectedGameObject->GetComponent<TransformComponent>()->transform().pos() + vec3(0, 1, 4);
		mainCamera.GetComponent<CameraComponent>()->camera().transform().lookAt(selectedGameObject->GetComponent<TransformComponent>()->transform().pos());
	}
	if (state[SDL_SCANCODE_LALT]) {
		altKeyPressed = true;
	}
	else {
		altKeyPressed = false;
	}

	mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();

	/*if (leftMousePressed) {

		if (leftMousePressed && state[SDL_SCANCODE_LALT]) {
			glm::vec2 mouseScreenPos = getMousePosition();
			ivec2 delta = ivec2(mouseScreenPos) - lastMousePosition; 
			lastMousePosition = ivec2(mouseScreenPos); 

			orbitCamera(target, delta.x, delta.y); 
		}
	}*/

}



static void mouseMotion_func(int x, int y) {
	if (middleMousePressed) {

		ivec2 delta = ivec2(x, y) - lastMousePosition;
		lastMousePosition = ivec2(x, y);


		mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(vec3(delta.x * 0.01, 0, -delta.y * 0.01));
		mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(vec3(0, delta.y * 0.01, 0));
	}

	if (leftMousePressed && altKeyPressed && selectedGameObject == nullptr) {

		int deltaX = x - lastMouseX;
		int deltaY = y - lastMouseY;

		// Actualizar la �ltima posici�n del rat�n
		if (deltaX == 0 && deltaY == 0) {
			return;
		}

		// Actualizar la �ltima posici�n del rat�n
		lastMouseX = x;
		lastMouseY = y;

		// Ajustar la sensibilidad seg�n el movimiento del rat�n para mayor suavidad
		const double baseSensitivity = 0.1;
		double movementMagnitude = glm::length(glm::vec2(deltaX, deltaY));
		const double adaptiveSensitivity = baseSensitivity * (1.0 + 0.01 * movementMagnitude);

		// Actualizar los �ngulos de rotaci�n en base al movimiento del rat�n
		yaw = initialYaw + deltaX * adaptiveSensitivity;
		pitch = initialPitch - deltaY * adaptiveSensitivity;

		// Limitar el �ngulo de inclinaci�n (pitch) para evitar que la c�mara se voltee
		if (pitch > MAX_PITCH) pitch = MAX_PITCH;
		if (pitch < -MAX_PITCH) pitch = -MAX_PITCH;

		// Aplicar las rotaciones a la c�mara
		mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(-deltaX * adaptiveSensitivity), glm::vec3(0, -1, 0));
		mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(deltaY * adaptiveSensitivity), glm::vec3(-1, 0, 0));

		// Realinear la c�mara despu�s de la rotaci�n
		mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();
	}
	else if (leftMousePressed && altKeyPressed && selectedGameObject != nullptr) {
		// Calcular el desplazamiento del rat�n
		int deltaX = x - lastMouseX;
		int deltaY = y - lastMouseY;

		// Actualizar la �ltima posici�n del rat�n
		lastMouseX = x;
		lastMouseY = y;

		// Ajustar la sensibilidad de movimiento
		const double baseSensitivity = 0.1;
		double movementMagnitude = glm::length(glm::vec2(deltaX, deltaY));
		const double adaptiveSensitivity = baseSensitivity * (1.0 + 0.01 * movementMagnitude);

		// Actualizar los �ngulos de rotaci�n para la �rbita
		yaw += deltaX * adaptiveSensitivity;
		pitch -= deltaY * adaptiveSensitivity;

		// Limitar el �ngulo de pitch para evitar que la c�mara se voltee
		if (pitch > MAX_PITCH) pitch = MAX_PITCH;
		if (pitch < -MAX_PITCH) pitch = -MAX_PITCH;

		// Convertir los �ngulos a radianes para el c�lculo de �rbita
		float radiansYaw = glm::radians(yaw);
		float radiansPitch = glm::radians(pitch);

		// Definir el radio de la �rbita
		float orbitRadius = 10.0f;

		// Calcular la nueva posici�n de la c�mara en la �rbita alrededor del objeto seleccionado
		glm::vec3 orbitPosition = glm::vec3(selectedGameObject->GetComponent<TransformComponent>()->transform().pos()) + glm::vec3(
			orbitRadius * cos(radiansPitch) * sin(radiansYaw),
			orbitRadius * sin(radiansPitch),
			orbitRadius * cos(radiansPitch) * cos(radiansYaw)
		);

		// Actualizar la posici�n de la c�mara y orientar hacia el objeto seleccionado
		mainCamera.GetComponent<CameraComponent>()->camera().transform().SetPosition(orbitPosition);
		mainCamera.GetComponent<CameraComponent>()->camera().transform().lookAt(selectedGameObject->GetComponent<TransformComponent>()->transform().pos());
	}
	
	if (rightMousePressed) {
		if (rightMousePressed) {
			// Calcular la diferencia de movimiento del rat�n
			int deltaX = x - lastMouseX;
			int deltaY = y - lastMouseY;

			if (deltaX != 0 || deltaY != 0) {
				// Ajustar la sensibilidad seg�n el movimiento del rat�n para mayor suavidad
				const double baseSensitivity = 0.1;
				double movementMagnitude = glm::length(glm::vec2(deltaX, deltaY));
				const double adaptiveSensitivity = baseSensitivity * (1.0 + 0.01 * movementMagnitude);

				// Actualizar los �ngulos de rotaci�n
				yaw = initialYaw + deltaX * adaptiveSensitivity;
				pitch = initialPitch - deltaY * adaptiveSensitivity;

				// Limitar el �ngulo de inclinaci�n (pitch) para evitar que la c�mara se voltee
				if (pitch > MAX_PITCH) pitch = MAX_PITCH;
				if (pitch < -MAX_PITCH) pitch = -MAX_PITCH;

				// Aplicar las rotaciones utilizando interpolaci�n suave
				mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(-deltaX * adaptiveSensitivity), glm::vec3(0, -1, 0));
				mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(deltaY * adaptiveSensitivity), glm::vec3(-1, 0, 0));

				// Guardar la �ltima posici�n del rat�n
				lastMouseX = x;
				lastMouseY = y;

				// Realinear la c�mara despu�s de la rotaci�n
				mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();
			}
		}
	}
	
}


std::string getFileExtension(const std::string& filePath) {
	// Find the last dot in the file path
	size_t dotPosition = filePath.rfind('.');

	// If no dot is found, return an empty string
	if (dotPosition == std::string::npos) {
		return "";
	}

	// Extract and return the file extension
	return filePath.substr(dotPosition + 1);
}

// Function to check if the mouse is over a GameObject
bool isMouseOverGameObject(const GameObject& go, int mouseX, int mouseY) {
	// Obtener la posici�n de la c�mara (origen del rayo)
	glm::vec3 rayOrigin = mainCamera.GetComponent<CameraComponent>()->camera().transform().pos();

	// Convertir las coordenadas del mouse a un rayo en el espacio del mundo
	glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY, WINDOW_SIZE.x, WINDOW_SIZE.y, viewMatrix, projectionMatrix);

	// Obtener el bounding box del GameObject
	BoundingBox bbox = go.boundingBox();

	// Verificar si el rayo intersecta con el bounding box
	return rayIntersectsBoundingBox(rayOrigin, rayDir, bbox);
}

int main(int argc, char* argv[]) {

	Log::getInstance().logMessage("Hello World!");
	Log::getInstance().logMessage("Hello World! 2");
	Log::getInstance().logMessage("Hello World! 3");
	ilInit();
	iluInit();
	ilutInit();
	MyWindow window("Badass Engine", WINDOW_SIZE.x, WINDOW_SIZE.y);
	MyGUI gui(window.windowPtr(), window.contextPtr());
	init_opengl();


	GameObject testCamera;
	testCamera.SetName("Test Camera");
	testCamera.AddComponent<CameraComponent>()->camera().setProjection(45.0, 1.0, 0.1, 100.0);
	testCamera.GetComponent<TransformComponent>()->transform().pos() = vec3(0, 1, 4);
	testCamera.GetComponent<TransformComponent>()->transform().rotate(glm::radians(180.0), vec3(0, 1, 0));
	scene.emplaceChild(testCamera);
	

	mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() = vec3(0, 1, 4);
	mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(180.0), vec3(0, 1, 0));
	scene.emplaceChild(mainCamera);

	SDL_Event event;
	char* dropped_filePath;
	auto mesh = make_shared<Mesh>();
	auto testMesh = make_shared<Mesh>();
	auto imageTexture = make_shared<Image>();
	auto texture = std::make_shared<Texture>();
	auto material = std::make_shared<Material>();
	std::string extension;

	scene.name = "Scene";

	GameObject go;
	FileManager fileManager;
	go = fileManager.LoadFile("Library/Assets/street2.FBX");
	scene.emplaceChild(go);

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	while (window.isOpen()) {
		projectionMatrix = mainCamera.GetComponent<CameraComponent>()->camera().projection();
		viewMatrix = mainCamera.GetComponent<CameraComponent>()->camera().view();
		GetMemoryUsage(gui);
		const auto t0 = hrclock::now();
		handleKeyboardInput();
		display_func();
		gui.render();
		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);

		while (SDL_PollEvent(&event))
		{
			window.processEvent(event);
			if (!window.isOpen()) break;
			gui.processEvent(event);

			switch (event.type) {
			case SDL_DROPFILE:
				dropped_filePath = event.drop.file;
				extension = getFileExtension(dropped_filePath);

				if (extension == "obj" || extension == "fbx" || extension == "dae") {
					mesh->LoadFile(dropped_filePath);
					GameObject go;
					go.meshPath = dropped_filePath;
					go.AddComponent<MeshLoader>()->SetMesh(mesh);
					go.setMesh(mesh);
					scene.emplaceChild(go);
				}
				else if (extension == "png" || extension == "jpg" || extension == "bmp") {
					int mouseX, mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);
					for (auto& child : scene.children()) {
						if (isMouseOverGameObject(child, mouseX, mouseY)) {
							imageTexture->LoadTexture(dropped_filePath);
							go.texturePath = dropped_filePath;
							texture->setImage(imageTexture);
							child.GetComponent<MeshLoader>()->GetMesh()->deleteCheckerTexture();
							child.GetComponent<MeshLoader>()->SetImage(imageTexture);
							child.GetComponent<MeshLoader>()->SetTexture(texture);
						}
					}

				}
				else {
					std::cerr << "Unsupported file extension: " << extension << std::endl;
				}
				SDL_free(dropped_filePath);
				//Hasta aqu�
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					int mouseX, mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);
					if (mouseX > 300 && mouseX < 900)
					{
						// Convertir las coordenadas del rat�n a coordenadas del mundo
						selectedGameObject = raycastFromMouseToGameObject(event.button.x, event.button.y, projectionMatrix, viewMatrix, WINDOW_SIZE);

					}
					
					
				}
			case SDL_MOUSEBUTTONUP:
				mouseButton_func(event.button.button, event.button.state, event.button.x, event.button.y);
				break;
			case SDL_MOUSEMOTION:
				mouseMotion_func(event.motion.x, event.motion.y);
				break;

			case SDL_MOUSEWHEEL:
				mouseWheel_func(event.wheel.y);
				break;
			}
		}

	}

	return EXIT_SUCCESS;
}
