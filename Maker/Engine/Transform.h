#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "types.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {

	union {
		mat4 _mat = mat4(1.0);
		struct {
			vec3 _left; mat4::value_type _left_w;
			vec3 _up; mat4::value_type _up_w;
			vec3 _fwd; mat4::value_type _fwd_w;
			vec3 _pos; mat4::value_type _pos_w;
		};
	};

public:
	

	const auto& mat() const { return _mat; }
	const auto& left() const { return _left; }
	const auto& up() const { return _up; }
	const auto& fwd() const { return _fwd; }
	const auto& pos() const { return _pos; }
	const auto& GetRotation() const {
		// Calculate the rotation matrix from the _left, _up, and _fwd vectors
		mat4 rotationMatrix = mat4(1.0);
		rotationMatrix[0] = vec4(_left, 0.0);
		rotationMatrix[1] = vec4(_up, 0.0);
		rotationMatrix[2] = vec4(_fwd, 0.0);

		// Extract Euler angles from the rotation matrix
		vec3 eulerAngles = glm::eulerAngles(glm::quat_cast(rotationMatrix));

		// Convert radians to degrees
		eulerAngles = glm::degrees(eulerAngles);

		return eulerAngles;
	}

	const auto& GetScale() const {

		glm::vec3 left(_mat[0][0], _mat[0][1], _mat[0][2]);
		glm::vec3 up(_mat[1][0], _mat[1][1], _mat[1][2]);
		glm::vec3 forward(_mat[2][0], _mat[2][1], _mat[2][2]);
		// Calculate the scale vector from the _left, _up, and _fwd vectors
		vec3 scale;
		scale.x = glm::length(left);
		scale.y = glm::length(up);
		scale.z = glm::length(forward);

		return scale;
	}

	auto& pos() { return _pos; }

	const auto* data() const { return &_mat[0][0]; }

	void translate(const vec3& v);
	void rotate(double rads, const vec3& v);
	void lookAt(const vec3& target);
	void SetPosition(const vec3& position);
	void alignCamera(const vec3& worldUp = vec3(0.0f, 1.0f, 0.0f));
	void SetRotation(const vec3& eulerAngles) {
		// Convert degrees to radians
		vec3 eulerAnglesRad = glm::radians(eulerAngles);

		// Convert Euler angles to quaternion
		glm::quat quaternion = glm::quat(eulerAnglesRad);

		// Convert quaternion to rotation matrix
		mat4 rotationMatrix = glm::toMat4(quaternion);

		// Calculate the new _left, _up, and _fwd vectors
		_left = vec3(rotationMatrix[0]);
		_up = vec3(rotationMatrix[1]);
		_fwd = vec3(rotationMatrix[2]);
	}
	void SetScale(const vec3& scale) {
		// Normalize the _left, _up, and _fwd vectors
		vec3 leftNormalized = glm::normalize(_left);
		vec3 upNormalized = glm::normalize(_up);
		vec3 fwdNormalized = glm::normalize(_fwd);

		// Scale the vectors by the provided scale
		_left = leftNormalized * scale.x;
		_up = upNormalized * scale.y;
		_fwd = fwdNormalized * scale.z;

		// Update the transformation matrix
		_mat[0] = vec4(_left, 0.0);
		_mat[1] = vec4(_up, 0.0);
		_mat[2] = vec4(_fwd, 0.0);
	}
};
