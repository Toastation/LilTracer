#pragma once
#include <lt/lt_common.h>
#include <lt/scene.h>
#include <lt/camera.h>
#include <lt/sensor.h>
#include <lt/sampler.h>

namespace LT_NAMESPACE {

	class Integrator
	{
	public:
		
		~Integrator() {};

		
		virtual void render(Camera* camera, Sensor* sensor, Scene& scene, Sampler& sampler) = 0;

	};

	class PolarSlice : public Integrator {
		PolarSlice() {};
		void render(Camera* camera, Sensor* sensor, Scene& scene, Sampler& sampler) {

		}
	};

	class NormalIntegrator : public Integrator 
	{
	public:
		NormalIntegrator() {};

		void render(Camera* camera, Sensor* sensor, Scene& scene, Sampler& sampler) {
			std::vector<float> u = linspace<Float>(-1, 1, sensor->w);
			std::vector<float> v = linspace<Float>(-1, 1, sensor->h);

			for (int i = 0; i < u.size(); i++) {
				for (int j = 0; j < v.size(); j++) {
					Ray r = camera->generate_ray(u[i], v[j]);
					SurfaceInteraction si;
					if (scene.intersect(r, si)) {
						sensor->data[j * sensor->h + i] = glm::abs(si.nor);
					}

				}
			}

		}
	};

	class DirectIntegrator : public Integrator
	{
	public:
		DirectIntegrator() {};

		void render(Camera* camera, Sensor* sensor, Scene& scene, Sampler& sampler) {
			std::vector<float> u = linspace<Float>(-1, 1, sensor->w);
			std::vector<float> v = linspace<Float>( 1,-1, sensor->h);

			float jx = (2. * sampler.next_float()) / (float)sensor->w;
			float jy = (2. * sampler.next_float()) / (float)sensor->h;

			for (int j = 0; j < v.size(); j++) {
				for (int i = 0; i < u.size(); i++) {
					Ray r = camera->generate_ray(u[i] + jx, v[j] + jy);
					SurfaceInteraction si;
					
					if (scene.intersect(r, si)) {

						if (!si.brdf)
							break;
						
						for (int l = 0; l < scene.lights.size(); l++) {

							vec3 ld = scene.lights[l]->sample_light_direction();
							
							vec3 n = si.nor;
							vec3 t = glm::normalize(glm::cross(n, vec3(0., 1., 0.)));
							t = glm::normalize(glm::cross(t, n));
							vec3 b = glm::normalize(glm::cross(t, n));
							glm::mat3 tbn = glm::mat3(t, b, n);
							glm::mat3 inv_tbn = glm::transpose(tbn);

							vec3 wi = inv_tbn * (-r.d);
							vec3 wo = inv_tbn * ld;

							sensor->data[j * u.size() + i] = si.brdf->eval(wi, wo);

						}

						
					}
				}
			}

		}
	};


}