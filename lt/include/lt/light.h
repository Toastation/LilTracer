/**
 * @file
 * @brief Definitions of the Light and DirectionalLight classes.
 */

#pragma once

#include <lt/factory.h>
#include <lt/geometry.h>
#include <lt/io_exr.h>
#include <lt/lt_common.h>
#include <lt/serialize.h>
#include <lt/texture.h>

namespace LT_NAMESPACE {

/**
 * @brief Abstract base class for light sources.
 */
    class Light : public Serializable {
    public:
        /**
         * @brief Constructor for Light.
         * @param type The type of light.
         */
        Light(const std::string& type)
            : Serializable(type)
        {
        }

        /**
         * @brief Pure virtual function for sampling light direction.
         */
        virtual void sample(const SurfaceInteraction& si, vec3& direction,
            vec3& emission, Float& pdf, Sampler& sampler)
            = 0;

        virtual Spectrum eval(const vec3& direction) = 0;

        virtual bool has_geometry();
        virtual int geometry_id();
};

/**
 * @brief Class representing a directional light source.
 */
class DirectionnalLight : public Light {
public:
    /**
     * @brief Default constructor for DirectionalLight.
     * Initializes the light type and default parameters.
     */
    DirectionnalLight()
        : Light("DirectionnalLight")
    {
        link_params();
    }

    /**
     * @brief Sample the direction of the light.
     * @return The direction of the light.
     */
    void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler);

    Spectrum eval(const vec3& direction);

    /**
     * @brief Initialize the directional light.
     * Normalize the direction vector.
     */
    void init();

    Float intensity = 0.5; /**< Intensity of the light. */
    vec3 dir = vec3(1, 0, 0); /**< Direction of the light. */

protected:
    /**
     * @brief Link parameters with the Params struct.
     */
    void link_params()
    {
        params.add("dir", Params::Type::VEC3, &dir);
        params.add("intensity", Params::Type::FLOAT, &intensity);
    }
};

/**
 * @brief Class representing a directional light source.
 */
class EnvironmentLight : public Light {
public:
    EnvironmentLight()
        : Light("EnvironmentLight")
    {
        link_params();
    }

    void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission,
        Float& pdf, Sampler& sampler);

    Spectrum eval(const vec3& direction);

    void compute_density();

    void init();

    Texture<Spectrum> envmap;
    Float intensity;

    Texture<Float> density;
    Texture<Float> cumulative_density;
    std::vector<Float> c;
    Float dtheta;
    Float dphi;

protected:
    void link_params()
    {
        params.add("texture", Params::Type::TEXTURE, &envmap);
        params.add("intensity", Params::Type::FLOAT, &intensity);
    }
};

class SphereLight : public Light {
public:
    SphereLight()
        : Light("SphereLight")
    {
        link_params();
    }

    glm::mat3 build_from_w(const vec3& w);

    void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler);

    Spectrum eval(const vec3& direction);

    bool has_geometry();
    int geometry_id();

    std::shared_ptr<Sphere> sphere;


protected:
    /**
     * @brief All param are from Sphere and Sphere::brdf.
     */
    void link_params() { }
};

} // namespace LT_NAMESPACE