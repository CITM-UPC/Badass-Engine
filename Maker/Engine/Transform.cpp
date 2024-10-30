#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>



void Transform::translate(const vec3& v) {
	_mat = glm::translate(_mat, v);
}

void Transform::rotate(double rads, const vec3& v) {
	_mat = glm::rotate(_mat, rads, v);
}

void Transform::alignCamera(const vec3& worldUp) {

    vec3 fwd = glm::normalize(_fwd);
    vec3 right = glm::normalize(glm::cross(worldUp, fwd));
    vec3 up = glm::cross(fwd, right);


    _left = right;
    _up = up;
    _fwd = fwd;
    _pos = _pos;
    _mat = mat4(vec4(_left, 0.0f), vec4(_up, 0.0f), vec4(_fwd, 0.0f), vec4(_pos, 1.0f));
}

void Transform::lookAt(const vec3& target) {
    _fwd = glm::normalize(_pos - target);
    _left = glm::normalize(glm::cross(vec3(0, 1, 0), _fwd));
    _up = glm::cross(_fwd, _left);
    _mat[0] = vec4(_left, 0.0);
    _mat[1] = vec4(_up, 0.0);
    _mat[2] = vec4(-_fwd, 0.0);
    _mat[3] = vec4(_pos, 1.0);
}

void Transform::SetPosition(const vec3& position) {
    _pos = position;
    // Actualizar la matriz de transformaci�n con la nueva posici�n
    _mat[3] = vec4(position, 1.0f);
}