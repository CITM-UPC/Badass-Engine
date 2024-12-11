#pragma once
#include <GL/glew.h>
#include <memory>
#include <string> 
#include <glm/glm.hpp>
#include "TreeExt.h"
#include "Transform.h"
#include "TransformComponent.h"
#include "Camera.h"
#include "CameraComponent.h"
#include "Texture.h"
#include "BoundingBox.h"
#include "Component.h"
#include "Mesh.h"
#include "Scene.h"

class GameObject : public std::enable_shared_from_this<GameObject>, public TreeExt<GameObject>
{

public:
	std::shared_ptr<Mesh> _mesh_ptr;
	glm::u8vec3 _color = glm::u8vec3(255, 255, 255);
	Texture _texture;
	
	int id = 0;
	std::string name;
	std::string meshPath;
	std::string texturePath;
	bool drawTexture = true;
	static int nextID; // Static member to keep track of the next available ID

	

	std::string tag = "Untagged";
	bool active = true;
	bool destroyed = false;
	std::vector<std::shared_ptr<GameObject>> _children;

	std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
	const std::vector<std::shared_ptr<GameObject>>& GetChildren() const {
		return _children;
	
	}
public:
	GameObject(const std::string& name = "GameObject");
	/*GameObject(const GameObject& other) = delete;
	GameObject& operator=(const GameObject& other) = delete;
	GameObject(GameObject&& other) = default;
	GameObject& operator=(GameObject&& other) = default;*/
	~GameObject();

	template <typename T, typename... Args>
	std::shared_ptr<T> AddComponent(Args&&... args);

	template <typename T>
	std::shared_ptr<T> GetComponent() const;

	template <typename T>
	void RemoveComponent();

	template <typename T>
	bool HasComponent() const;

	std::string GetName() const;
	void SetName(const std::string& name);

	bool CompareTag(const std::string& tag) const;

	bool operator==(const GameObject& other) const {
		return this->id == other.id;
	}

	int getId() const { return id; }
	const auto& color() const { return _color; }
	auto& color() { return _color; }
	const auto& texture() const { return _texture; }
	auto& texture() { return _texture; }
	const auto& mesh() const { return *_mesh_ptr; }
	auto& mesh() { return *_mesh_ptr; }

	BoundingBox boundingBox() const;
	BoundingBox localBoundingBox() const { return _mesh_ptr ? _mesh_ptr->boundingBox() : BoundingBox(); }

	void drawWiredQuad(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3);
	void drawBoundingBox(const BoundingBox& box);

	void setTextureImage(const std::shared_ptr<Image>& img_ptr) { _texture.setImage(img_ptr); }
	void setMesh(const std::shared_ptr<Mesh>& mesh_ptr) { _mesh_ptr = mesh_ptr; }

	bool hasTexture() const { return _texture.id(); }
	bool hasMesh() const { return _mesh_ptr != nullptr; }

	// Basic Shapes
	static GameObject createCube();
	static GameObject createSphere();
	static GameObject createCylinder();

	// Functions for the Hierarchy
	static GameObject CreateEmpty();
	static GameObject CreateEmptyChild(GameObject& parent);
	static GameObject ReparentGameObject(GameObject& newParent, GameObject& child);
	void RemoveAsChild();
	void DeleteGameObject();

	void draw() const;
	void drawAxis(double size);
	void drawDebug(const GameObject& obj);

	void UpdateCamera() const;

private:
	
	
	mutable std::type_index cachedComponentType;
	mutable std::shared_ptr<Component> cachedComponent;
};

template <typename T, typename... Args>
std::shared_ptr<T> GameObject::AddComponent(Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
	std::shared_ptr<T> newComponent = std::make_shared<T>(weak_from_this(), std::forward<Args>(args)...);
	components[typeid(T)] = newComponent;
	return newComponent;
}

template <typename T>
std::shared_ptr<T> GameObject::GetComponent() const {
	if (this == nullptr) {
		throw std::runtime_error("GameObject instance is null");
	}
	if (cachedComponentType == typeid(T) && cachedComponent) {
		return std::static_pointer_cast<T>(cachedComponent);
	}

	auto it = components.find(typeid(T));
	if (it != components.end()) {
		cachedComponentType = typeid(T);
		cachedComponent = it->second;
		return std::static_pointer_cast<T>(cachedComponent);
	}
	else {
		throw std::runtime_error("Component not found on GameObject: " + this->GetName());
	}
}

template <typename T>
void GameObject::RemoveComponent() {
	auto it = components.find(typeid(T));
	if (it != components.end()) {
		components.erase(it);
	}
	else {
		//Log a warning 
	}
}

template <typename T>
bool GameObject::HasComponent() const {
	return components.find(typeid(T)) != components.end();
}
