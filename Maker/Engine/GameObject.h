#pragma once

#include <memory>
#include <string> 
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "TreeExt.h"
#include "Transform.h"
#include "Texture.h"
#include "BoundingBox.h"
#include "Mesh.h"

class GameObject : public TreeExt<GameObject>
{
public:
	Transform _transform;
	glm::u8vec3 _color = glm::u8vec3(255, 255, 255);
	Texture _texture;
	std::shared_ptr<Mesh> _mesh_ptr;
	std::string name = "GameObject";
	bool drawTexture = true;

public:
	const auto& transform() const { return _transform; }
	auto& transform() { return _transform; }
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
	static std::shared_ptr<GameObject> createCube();
	static std::shared_ptr<GameObject> createSphere();
	static std::shared_ptr<GameObject> createCylinder();

	void draw() const;
	void drawAxis(double size);
	void drawDebug(const GameObject& obj);
};
