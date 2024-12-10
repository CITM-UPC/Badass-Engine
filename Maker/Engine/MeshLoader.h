#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include "Material.h"

class Mesh;
class Texture;
class Image;

class MeshLoader : public Component {
public:
    explicit MeshLoader(std::weak_ptr<GameObject> owner);
    ~MeshLoader() override = default;

    void SetMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> GetMesh() const;

    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;

    void SetImage(std::shared_ptr<Image> image);
    std::shared_ptr<Image> GetImage() const;

	void SetMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetMaterial() const;

    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;

    void Render() const;

    

private:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Image> image;
    std::shared_ptr<Texture> texture;
	std::shared_ptr<Material> material;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

public:
	bool drawNormals = false;
    bool drawTexture = true;
};

