#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"

class TransformComponent : public Component 
{
	Transform _transform;
public:
	explicit TransformComponent(std::weak_ptr<GameObject> owner) : Component(owner) {}
	const auto& transform() const { return _transform; }
	auto& transform() { return _transform; }

	void setTransform(const glm::mat4& mat) {
		_transform = mat;
	}
};

