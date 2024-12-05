#pragma once

#include "Transform.h"

struct Plane
{
    //plane eq -> ax + by + cz + d = 0
    glm::vec3 normal;
    float distance;

    static Plane CreatePlaneFromVec4(glm::vec4 vec)
    {
        Plane plane;
        plane.normal.x = vec.x;
        plane.normal.y = vec.y;
        plane.normal.z = vec.z;
        plane.distance = vec.w;
        return plane;
    }

    void Normalise()
    {
        float magnitude = glm::length(normal);
        normal /= magnitude;
        distance /= magnitude;
    }
};

struct Frustum : public Plane
{
    // hekbas - don't use near and far as those are already defined in minwindef.h
    Plane _near;
    Plane _far;
    Plane left;
    Plane right;
    Plane top;
    Plane bot;

    glm::vec3 vertices[8];

    void Update(const glm::mat4& vpm)
    {
        //glm::mat4 vpm = glm::mat4(1.0f);

        left = CreatePlaneFromVec4({ vpm[0][3] + vpm[0][0], vpm[1][3] + vpm[1][0], vpm[2][3] + vpm[2][0], vpm[3][3] + vpm[3][0] });
        right = CreatePlaneFromVec4({ vpm[0][3] - vpm[0][0], vpm[1][3] - vpm[1][0], vpm[2][3] - vpm[2][0], vpm[3][3] - vpm[3][0] });
        bot = CreatePlaneFromVec4({ vpm[0][3] + vpm[0][1], vpm[1][3] + vpm[1][1], vpm[2][3] + vpm[2][1], vpm[3][3] + vpm[3][1] });
        top = CreatePlaneFromVec4({ vpm[0][3] - vpm[0][1], vpm[1][3] - vpm[1][1], vpm[2][3] - vpm[2][1], vpm[3][3] - vpm[3][1] });
        _near = CreatePlaneFromVec4({ vpm[0][3] + vpm[0][2], vpm[1][3] + vpm[1][2], vpm[2][3] + vpm[2][2], vpm[3][3] + vpm[3][2] });
        _far = CreatePlaneFromVec4({ vpm[0][3] - vpm[0][2], vpm[1][3] - vpm[1][2], vpm[2][3] - vpm[2][2], vpm[3][3] - vpm[3][2] });

        left.Normalise();
        right.Normalise();
        bot.Normalise();
        top.Normalise();
        _near.Normalise();
        _far.Normalise();

        CalculateVertices(vpm);
    }

    void CalculateVertices(const glm::mat4& transform)
    {
        static const bool zerotoOne = false;

        glm::mat4 transformInv = glm::inverse(transform);

        vertices[0] = glm::vec4(-1.0f, -1.0f, zerotoOne ? 0.0f : -1.0f, 1.0f);
        vertices[1] = glm::vec4(1.0f, -1.0f, zerotoOne ? 0.0f : -1.0f, 1.0f);
        vertices[2] = glm::vec4(1.0f, 1.0f, zerotoOne ? 0.0f : -1.0f, 1.0f);
        vertices[3] = glm::vec4(-1.0f, 1.0f, zerotoOne ? 0.0f : -1.0f, 1.0f);

        vertices[4] = glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
        vertices[5] = glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
        vertices[6] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        vertices[7] = glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);

        glm::vec4 temp;
        for (int i = 0; i < 8; i++)
        {
            temp = transformInv * glm::vec4(vertices[i], 1.0f);
            vertices[i] = temp / temp.w;
        }
    }
};

class Camera {

public:
	double fov = glm::radians(60.0);
	double aspect = 16.0 / 9.0;
	double zNear = 0.1;
	double zFar = 128.0;

private:
	Transform _transform;

public:
    bool drawFrustum = true;

	const auto& transform() const { return _transform; }
	auto& transform() { return _transform; }
    void UpdateCamera();

    Frustum frustum;
	mat4 projection() const;
	mat4 view() const;
	mat4 viewProjection() const;
	void setProjection(double fov, double aspect, double zNear, double zFar);
	

};
