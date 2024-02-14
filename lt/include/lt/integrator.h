#pragma once
#include <lt/lt_common.h>
#include <lt/scene.h>
#include <lt/camera.h>
#include <lt/sensor.h>

namespace LT_NAMESPACE {

	class Integrator
	{
	public:
		
		~Integrator() {};

		
		virtual void render(Camera* camera, Sensor* sensor, Scene& scene) = 0;

	};



	class NormalIntegrator : public Integrator 
	{
	public:
		NormalIntegrator() {};

		void render(Camera* camera, Sensor* sensor, Scene& scene) {
			std::vector<float> u = linspace<Float>(-1, 1, sensor->w);
			std::vector<float> v = linspace<Float>(-1, 1, sensor->h);

			for (int i = 0; i < u.size(); i++) {
				for (int j = 0; j < v.size(); j++) {
					Ray r = camera->generate_ray(u[i], v[j]);
					SurfaceInteraction si;
					if (scene.intersect(r, si)) {
						sensor->data[i * sensor->w + j] = glm::abs(si.nor);
					}

				}
			}

		}
	};

}