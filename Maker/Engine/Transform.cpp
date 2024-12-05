#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>




const vec3& Transform::GetRotation() const
{
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

const vec3& Transform::GetScale() const
{
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

void Transform::SetRotation(const vec3& eulerAngles)
{
    // Convert degrees to radians
    vec3 eulerAnglesRad = glm::radians(eulerAngles);

    // Convert Euler angles to quaternion
    glm::quat quaternion = glm::quat(eulerAnglesRad);

    // Convert quaternion to rotation matrix
    mat4 rotationMatrix = glm::toMat4(quaternion);

    // Calculate the new _left, _up, and _fwd vectors and normalize them
    _left = glm::normalize(vec3(rotationMatrix[0]));
    _up = glm::normalize(vec3(rotationMatrix[1]));
    _fwd = glm::normalize(vec3(rotationMatrix[2]));
}

void Transform::SetScale(const vec3& scale)
{
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
    // Actualizar la matriz de transformación con la nueva posición
    _mat[3] = vec4(position, 1.0f);
}