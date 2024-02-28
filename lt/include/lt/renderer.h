/**
 * @file
 * @brief Definition of the Renderer class.
 */

#pragma once
#include <lt/camera.h>
#include <lt/integrator.h>
#include <lt/lt_common.h>
#include <lt/sampler.h>
#include <lt/sensor.h>

namespace LT_NAMESPACE {

/**
 * @brief Class for managing rendering process.
 */
class Renderer {
 public:
  std::shared_ptr<Sampler> sampler;       /**< Pointer to the sampler. */
  std::shared_ptr<Sensor> sensor;         /**< Pointer to the sensor. */
  std::shared_ptr<Camera> camera;         /**< Pointer to the camera. */
  std::shared_ptr<Integrator> integrator; /**< Pointer to the integrator. */
};

}  // namespace LT_NAMESPACE