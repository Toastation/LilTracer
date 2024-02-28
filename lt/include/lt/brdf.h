/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>
#include <lt/sampler.h>
#include <lt/serialize.h>

namespace LT_NAMESPACE {
#define PARAMETER(type, name, default_values) type name = type(default_values)

/**
 * @brief Base class for Bidirectional Reflectance Distribution Functions
 * (BRDFs).
 */
class Brdf : public Serializable {
 public:
  /**
   * @brief Constructor.
   * @param type The type of the BRDF.
   */
  Brdf(const std::string &type) : Serializable(type){};

  /**
   * @brief Evaluates the BRDF.
   * @param wi Incident direction.
   * @param wo Outgoing direction.
   * @return The evaluated spectrum.
   */
  virtual Spectrum eval(vec3 wi, vec3 wo) = 0;

  /**
   * @brief Samples the BRDF.
   * @param wi Incident direction.
   * @param sampler The sampler object used for sampling.
   * @return The sampled direction.
   */
  virtual vec3 sample(const vec3 &wi, Sampler &sampler);

  /**
   * @brief Computes the density of a sample.
   * @param w Direction.
   * @return The density value.
   */
  virtual float pdf(vec3 w);
};

/**
 * @brief Diffuse BRDF class.
 */
class Diffuse : public Brdf {
 public:
  PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

  Diffuse() : Brdf("Diffuse") { link_params(); }

  Spectrum eval(vec3 wi, vec3 wo) {
    return albedo / pi * glm::clamp(wo[2], 0.f, 1.f);
  }

 protected:
  void link_params() { params.add("albedo", Params::Type::VEC3, &albedo); }
};

/**
 * @brief Rough Conductor BRDF class.
 */
class RoughConductor : public Brdf {
 public:
  PARAMETER(Float, alpha_x, 0.5); /**< Roughness value in x-direction. */
  PARAMETER(Float, alpha_y, 0.5); /**< Roughness value in y-direction. */

  RoughConductor() : Brdf("RoughConductor") { link_params(); }

  static Float D_ggx(const vec3 &wh, const Float &ax, const Float &ay) {
    return wh[2];
  }

  static Float G1_ggx(const vec3 &w, const Float &ax, const Float &ay) {
    return 1.;
  }

  Spectrum eval(vec3 wi, vec3 wo) {
    vec3 wh = glm::normalize(wi + wo);

    Float D = D_ggx(wh, alpha_x, alpha_y);
    Float G = G1_ggx(wi, alpha_x, alpha_y) * G1_ggx(wi, alpha_x, alpha_y);
    return vec3(
        D * G /
        (4. * glm::clamp(wi[2], 0.f, 1.f) * glm::clamp(wo[2], 0.f, 1.f)));
  }

 protected:
  void link_params() {
    params.add("alpha_x", Params::Type::FLOAT, &alpha_x);
    params.add("alpha_y", Params::Type::FLOAT, &alpha_y);
  }
};

/**
 * @brief Test BRDF class for demonstration purposes.
 */
class TestBrdf : public Brdf {
 public:
  PARAMETER(float, v1, 0.5);            /**< Test parameter 1. (float) */
  PARAMETER(Spectrum, v2, 0.5);         /**< Test parameter 2. (Spectrum) */
  PARAMETER(std::vector<float>, v3, 0); /**< Test parameter 3. (Array<float>) */

  TestBrdf() : Brdf("TestBrdf") {
    v3.push_back(2.);
    v3.push_back(1.);
    v3.push_back(3.);
    v3.push_back(4.);
    v3.push_back(9.);
    link_params();
  }

  Spectrum eval(vec3 wi, vec3 wo) { return Spectrum(wo.z); }

 protected:
  void link_params() {
    params.add("float", Params::Type::FLOAT, &v1);
    params.add("vec3", Params::Type::VEC3, &v2);
    params.add("array", Params::Type::SH, &v3);
  }
};

}  // namespace LT_NAMESPACE