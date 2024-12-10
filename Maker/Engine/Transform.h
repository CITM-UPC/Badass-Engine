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
	const vec3& GetRotation() const;

	const vec3& GetScale() const;

	auto& pos() { return _pos; }

	const auto* data() const { return &_mat[0][0]; }

	void translate(const vec3& v);
	void rotate(double rads, const vec3& v);
	void lookAt(const vec3& target);
	void SetPosition(const vec3& position);
	void alignCamera(const vec3& worldUp = vec3(0.0f, 1.0f, 0.0f));
	void SetRotation(const vec3& eulerAngles);
	void SetScale(const vec3& scale);

	Transform& operator=(const glm::mat4& mat) {
		_mat = mat;
		return *this;
	}
};
