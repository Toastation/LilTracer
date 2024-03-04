/**
 * @file
 * @brief Definitions of the Light and DirectionalLight classes.
 */

#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>
#include <lt/serialize.h>
#include <lt/texture.h>
#include <lt/io_exr.h>

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
  Light(const std::string &type) : Serializable(type) {}

  /**
   * @brief Pure virtual function for sampling light direction.
   * @return The sampled light direction.
   */
  virtual vec3 sample_light_direction(Sampler& sampler) = 0;
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
  DirectionnalLight() : Light("DirectionnalLight") { link_params(); }

  /**
   * @brief Sample the direction of the light.
   * @return The direction of the light.
   */
  vec3 sample_light_direction(Sampler& sampler) { return dir; }

  /**
   * @brief Initialize the directional light.
   * Normalize the direction vector.
   */
  void init() { dir = glm::normalize(dir); }

  Float intensity = 0.5;    /**< Intensity of the light. */
  vec3 dir = vec3(1, 0, 0); /**< Direction of the light. */

 protected:
  /**
   * @brief Link parameters with the Params struct.
   */
  void link_params() {
    params.add("dir", Params::Type::VEC3, &dir);
    params.add("intensity", Params::Type::FLOAT, &intensity);
  }
};



/**
 * @brief Class representing a directional light source.
 */
class EnvironmentLight : public Light {
public:
    EnvironmentLight() : Light("EnvironmentLight") { 
        link_params();
    }

    vec3 sample_light_direction(Sampler& sampler) { return square_to_uniform_hemisphere(sampler.next_float(), sampler.next_float()); }
    
    void init() { 
        load_texture_exr("kloofendal_48d_partly_cloudy_puresky_1k.exr", texture);
    }

    Texture texture;

protected:
    void link_params() {
    }
};

}  // namespace LT_NAMESPACE