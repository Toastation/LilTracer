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
#include <lt/geometry.h>

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
   */
  virtual void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler) = 0;

  virtual Spectrum eval(const vec3& direction) = 0;

  virtual bool has_geometry() { return false; }
  virtual int geometry_id() { return -1; }
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
  void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler) {
      direction = dir;
      emission = Spectrum(intensity);
      pdf = 1;
  }

  Spectrum eval(const vec3& direction) {
      return Spectrum(intensity);
  }

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

    void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler) {

#if 0
        direction = square_to_uniform_sphere(sampler.next_float(), sampler.next_float());
        pdf = square_to_uniform_sphere_pdf();
#endif // 0
#if 1
        Float u = sampler.next_float();
        
        int id = std::distance(c.begin(), std::lower_bound(c.begin(), c.end(), u));
        
        int y = id / envmap.w;
        y = envmap.h - y - 1;
        int x = id % envmap.w;

        Float theta = pi * ((Float)y + 0.5) / (Float)envmap.h;
        Float phi = pi + 2. * pi * ((Float)x + 0.5) / (Float)envmap.w;

        theta += dtheta * (sampler.next_float() - 0.5 );
        phi += dphi * (sampler.next_float() - 0.5);


        pdf = density.get(x, y);
        direction = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
#endif
        emission = eval(direction);
    }
    
    Spectrum eval(const vec3& direction) {
        Float u = (glm::atan(direction.z, direction.x) + pi) / (2 * pi);
        Float v = glm::acos(direction.y) / pi;
        return envmap.eval(u, v) * intensity;
    }

    
    void compute_density() {

        // Compute density
        density.w = envmap.w;
        density.h = envmap.h;
        density.initialize();

        for (int y = 0; y < envmap.h; y++) {
            Float theta = pi * ((Float)y + 0.5) / (Float)envmap.h;
            Float sin_theta = std::sin(theta);
            Float solid_angle = sin_theta * dphi * dtheta;
            for (int x = 0; x < envmap.w; x++) {
                Spectrum s = envmap.get(x, y);
                Float mean = (s.r + s.g + s.b) * 0.333333f;
                density.set(x, y, mean * solid_angle);
            }
        }

        // Compute cumulative density
        cumulative_density.w = envmap.w;
        cumulative_density.h = envmap.h;
        cumulative_density.initialize();
        cumulative_density.data[0] = density.data[0];
        for (int n = 1; n < envmap.w *envmap.h; n++) {
            cumulative_density.data[n] = cumulative_density.data[n - 1] + density.data[n];
        }

        
        // Normalize density
        for (int n = 0; n < envmap.w * envmap.h; n++) {
            density.data[n] /= cumulative_density.data[envmap.w * envmap.h - 1];
            cumulative_density.data[n] /= cumulative_density.data[envmap.w * envmap.h - 1];
        }

    }

    void init() {
        dtheta = pi / (Float)envmap.h;
        dphi = 2. * pi / (Float)envmap.w;

        compute_density();
        c = std::vector<float>(cumulative_density.data, cumulative_density.data + cumulative_density.w * cumulative_density.h);
    }

    
    Texture<Spectrum> envmap;
    Float intensity;

    Texture<Float> density;
    Texture<Float> cumulative_density;
    std::vector<Float> c;
    Float dtheta;
    Float dphi;

protected:
    void link_params() {
        params.add("texture", Params::Type::TEXTURE, &envmap);
        params.add("intensity", Params::Type::FLOAT, &intensity);
    }
};



class SphereLight : public Light {
public:

    SphereLight() : Light("SphereLight") { link_params(); }


    glm::mat3 build_from_w(const vec3& w) {
        vec3 unit_w = glm::normalize(w);
        vec3 a = (std::abs(unit_w.x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        vec3 v = glm::normalize(glm::cross(unit_w, a));
        vec3 u = glm::normalize(glm::cross(unit_w, v));
        return (glm::mat3(u, v, unit_w));
    }

    void sample(const SurfaceInteraction& si, vec3& direction, vec3& emission, Float& pdf, Sampler& sampler) {
        
#if 1
        vec3 point = square_to_uniform_sphere(sampler.next_float(), sampler.next_float());
        vec3 point_on_surface = point * sphere->rad + sphere->pos;
        
        direction = si.pos - point_on_surface;
        Float distance = glm::length(direction);
        direction /= distance;
        
        Float light_cosine = glm::dot(si.nor, -direction);
        Float light_area = 4. * pi * sphere->rad * sphere->rad;
        pdf = distance * distance / (light_area * light_cosine);
#endif

#if 0
        direction = sphere->pos - si.pos;
        Float distance = glm::length(direction);
        direction /= distance;

        Float cos_theta_max = std::sqrt(1. - sphere->rad * sphere->rad / (distance * distance));
        Float solid_angle = 2. * pi * (1 - cos_theta_max);
        pdf = 1. / solid_angle;


        // random to sphere 
        Float z = 1 + sampler.next_float() * (cos_theta_max - 1);

        auto phi = 2 * pi * sampler.next_float();
        auto x = cos(phi) * sqrt(1 - z * z);
        auto y = sin(phi) * sqrt(1 - z * z);
        vec3 local = vec3(x, y, z);

        glm::mat3 uvw = build_from_w(direction);
        direction = -glm::normalize(uvw * local);
#endif

        emission = sphere->brdf->emission();
    }

    Spectrum eval(const vec3& direction) {
        return sphere->brdf->emission();
    }

    std::shared_ptr<Sphere> sphere;

    bool has_geometry() { return true; }
    int geometry_id() { return sphere->rtc_id; }
    
protected:
    /**
     * @brief All param are from Sphere and Sphere::brdf.
     */
    void link_params() {}
};


}  // namespace LT_NAMESPACE