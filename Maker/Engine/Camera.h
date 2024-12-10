#pragma once
#include "types.h"

#include <array>
#include "Transform.h"
#include "BoundingBox.h"

enum PlaneSide
{
    BEHIND = -1,
    ON_PLANE = 0,
    IN_FRONT = 1
};

enum FrustumContainment
{
    FRUSTUM_OUT = 0,
    FRUSTUM_IN = 1,
    INTERSECT = 2
};

struct Plane
{
    //plane eq -> ax + by + cz + d = 0
    glm::vec3 normal{};
    float distance{};

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

    PlaneSide SideOfPlane(const glm::vec3& point) const
    {
        float distanceToPoint = glm::dot(normal, point) + distance;
        if (distanceToPoint < 0)
        {
            return BEHIND;
        }
        else if (distanceToPoint > 0)
        {
            return IN_FRONT;
        }
        else
        {
            return ON_PLANE;
        }
    }
};

struct Frustum : public Plane
{
    // hekbas - don't use near and far as those are already defined in minwindef.h
    Plane _near{};
    Plane _far{};
    Plane left{};
    Plane right{};
    Plane top{};
    Plane bot{};

    Plane* m_plane[6] = { &left, &right, &top, &bot, &_near, &_far };

    glm::vec3 vertices[8]{};

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

    // tests if a AaBox is within the frustrum
    int ContainsBBox(BoundingBox refBox) const
    {
        int iTotalIn = 0;
        // get the corners of the box into the vCorner array

        // test all 8 corners against the 6 sides
        // if all points are behind 1 specific plane, we are out
        // if we are in with all points, then we are fully in for(int p = 0; p < 6; ++p) {

        for (int p = 0; p < 6; ++p) {

            int iInCount = 8;
            int iPtIn = 1;
            std::array<vec3, 8> v = refBox.vertices();

            for (int i = 0; i < 8; ++i) {

                // test this point against the planes
                if (m_plane[p]->SideOfPlane(v[i]) == BEHIND) {
                    iPtIn = 0;
                    --iInCount;
                }
            }

            // were all the points outside of plane p?
            if (iInCount == 0)
                return(FRUSTUM_OUT);

            // check if they were all on the right side of the plane
            iTotalIn += iPtIn;
        }

        // so if iTotalIn is 6, then all are inside the view
        if (iTotalIn == 6)
            return(FRUSTUM_IN);

        // we must be partly in then otherwise
        return(INTERSECT);
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
    void UpdateCamera(Transform transform);
    void UpdateMainCamera();

    Frustum frustum;
    mat4 projection() const;
    mat4 view() const;
    mat4 viewProjection() const;

    mat4 viewMatrix{};
    mat4 projectionMatrix{};
    mat4 viewProjectionMatrix{};

    void UpdateProjection();
    void UpdateView(Transform transform);
    void UpdateViewProjection();
    void setProjection(double fov, double aspect, double zNear, double zFar);
};
