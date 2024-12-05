#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

glm::dmat4 Camera::projection() const {
	return projectionMatrix;
}

glm::dmat4 Camera::view() const {
	return viewMatrix;
}

glm::dmat4 Camera::viewProjection() const {
	return viewProjectionMatrix;
}

void Camera::setProjection(double fov, double aspect, double zNear, double zFar) {
	this->fov = fov;
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;
}

void Camera::UpdateCamera(Transform transform)
{
	UpdateProjection();
	UpdateView(transform);
	UpdateViewProjection();
	frustum.Update(viewProjectionMatrix);
}

void Camera::UpdateMainCamera()
{
	UpdateProjection();
	UpdateView(_transform);
	UpdateViewProjection();
	frustum.Update(viewProjectionMatrix);
}

void Camera::UpdateProjection()
{
	projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
}

void Camera::UpdateView(Transform transform)
{
	viewMatrix = glm::lookAt(transform.pos(), transform.pos() + transform.fwd(), transform.up());
}

void Camera::UpdateViewProjection()
{
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

