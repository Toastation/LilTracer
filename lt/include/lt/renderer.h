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
#include <lt/scene.h>
#include <thread>

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

  float render(Scene& scene) {
	  return integrator->render(camera, sensor, scene, *sampler);
  }

  void reset() {
	  sensor->reset();
  }

};

/**
 * @brief Class for managing async rendering process.
 */
class RendererAsync : public Renderer {
public:
	std::thread thr;
	bool need_reset;

	RendererAsync(){
		need_reset = false;
	}

	~RendererAsync() {
		//thr.join();
		thr.detach();
	}

	void reset() {
		need_reset = true;
	}

	bool render(Scene& scene) {
		if (thr.joinable()) {
			return false;
		}
		else {
			if (need_reset) {
				Renderer::reset();
				need_reset = false;
			}
			thr = std::thread([&](auto s) { Renderer::render(s); thr.detach(); }, scene);
			return true;
		}
	}
};

}  // namespace LT_NAMESPACE