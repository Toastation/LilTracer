/**
 * @file
 * @brief Definitions of the Camera and PerspectiveCamera classes.
 */

#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/serialize.h>

namespace LT_NAMESPACE {

/**
 * @brief Abstract base class for camera objects.
 */
class Camera : public Serializable {
public:
    /**
     * @brief Constructor for Camera.
     * @param type The type of camera.
     */
    Camera(const std::string& type)
        : Serializable(type)
    {
    }

    /**
     * @brief Pure virtual function for generating a ray from the camera.
     * @param u The horizontal coordinate of the point on the image plane
     * (normalized between -1 and 1).
     * @param v The vertical coordinate of the point on the image plane
     * (normalized between -1 and 1).
     * @return The generated ray.
     */
    virtual Ray generate_ray(Float u, Float v) = 0;
};

/**
 * @brief Class representing a perspective camera.
 */
class PerspectiveCamera : public Camera {
public:
    /**
     * @brief Default constructor for PerspectiveCamera.
     * Initializes the camera type and default parameters.
     */
    PerspectiveCamera()
        : Camera("PerspectiveCamera")
        , pos(vec3(-1., 0., 0.))
        , center(vec3(0))
        , fov(40.)
        , aspect(1.)
    {
        link_params();
    }

    /**
     * @brief Initialize the perspective camera.
     * Computes view and projection matrices.
     */
    void init()
    {
        view = glm::lookAt(pos, center, vec3(0., 1., 0.));
        inv_view = glm::inverse(view);
        proj = glm::perspective((double)fov * pi / 180.f, (double)aspect, 0.0001, 1.);
        inv_proj = glm::inverse(proj);
    }

    /**
     * @brief Generate a ray from the perspective camera.
     * @param u The horizontal coordinate of the point on the image plane
     * (normalized between -1 and 1).
     * @param v The vertical coordinate of the point on the image plane
     * (normalized between -1 and 1).
     * @return The generated ray.
     */
    Ray generate_ray(Float u, Float v)
    {
        glm::vec4 d_eye = inv_proj * glm::vec4(u, v, -1., 1.);
        d_eye.w = 0.;

        vec3 d = glm::vec3(inv_view * d_eye);
        d = glm::normalize(d);

        return Ray(pos, d);
    }

    vec3 pos; /**< Camera position. */
    vec3 center; /**< Target position. */
    float fov; /**< Field of view angle (in degrees). */
    float aspect; /**< Aspect ratio. */
    glm::mat4 view; /**< View matrix. */
    glm::mat4 proj; /**< Projection matrix. */
    glm::mat4 inv_view; /**< Inverse of view matrix. */
    glm::mat4 inv_proj; /**< Inverse of projection matrix. */

protected:
    /**
     * @brief Link parameters with the Params struct.
     */
    void link_params()
    {
        params.add("pos", Params::Type::VEC3, &pos);
        params.add("center", Params::Type::VEC3, &center);
        params.add("aspect", Params::Type::FLOAT, &aspect);
        params.add("fov", Params::Type::FLOAT, &fov);
    }
};

} // namespace LT_NAMESPACE
