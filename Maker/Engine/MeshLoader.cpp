#include <GL/glew.h>
#include "MeshLoader.h"
#include "GameObject.h" 
#include "Transform.h"
#include "Mesh.h" 
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Image.h"

MeshLoader::MeshLoader(std::weak_ptr<GameObject> owner) : Component(owner) {}

void MeshLoader::SetMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
}

std::shared_ptr<Mesh> MeshLoader::GetMesh() const
{
    return mesh;
}

void MeshLoader::SetColor(const glm::vec3& color)
{
    this->color = color;
}

glm::vec3 MeshLoader::GetColor() const
{
    return color;
}

void MeshLoader::SetTexture(std::shared_ptr<Texture> texture)
{
    this->texture = texture;
}

std::shared_ptr<Texture> MeshLoader::GetTexture() const
{
    return texture;
}

void MeshLoader::SetImage(std::shared_ptr<Image> image)
{
	this->image = image;
}

std::shared_ptr<Image> MeshLoader::GetImage() const
{
    return image;
}

void MeshLoader::SetMaterial(std::shared_ptr<Material> material)
{
	this->material = material;
}

std::shared_ptr<Material> MeshLoader::GetMaterial() const
{
	return material;
}

void MeshLoader::Render() const
{
    /*if (material)
    {
        if (material->texture.id() && drawTexture)
        {
            glEnable(GL_TEXTURE_2D);
            material->texture.bind();
        }
    }
	else if (!material || !drawTexture)
	{
		mesh->CheckerTexture();
	}

    if (mesh) mesh->draw();
    if (drawNormals) mesh->drawNormals(0.1f);

    if (material && material->texture.id())
    {
        glDisable(GL_TEXTURE_2D);
    }*/
    if (material) {
        glColor4ubv(&material->color.r);
        if (material->texture.id()) {
            glEnable(GL_TEXTURE_2D);
            material->texture.bind();
        }
    }

    if (mesh) mesh->draw();

    if (material && material->texture.id()) glDisable(GL_TEXTURE_2D);
}
