#pragma once
#include <lt/lt_common.h>
#include <lt/serialize.h>
#include <lt/scene.h>
#include <lt/camera.h>
#include <lt/sensor.h>
#include <lt/sampler.h>

namespace LT_NAMESPACE {

	class Integrator : public Serializable
	{
	public:
		
		Integrator(const std::string& type) : Serializable(type) {};

		virtual void render(std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) = 0;

	};


	class DirectIntegrator : public Integrator
	{
	public:
		DirectIntegrator() : Integrator("DirectIntegrator") {
			link_params();
		};

		void render(std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) {
			std::vector<Float> u = linspace<Float>(-1, 1, sensor->w);
			std::vector<Float> v = linspace<Float>(-1, 1, sensor->h);

			float jx = (2. * sampler.next_float()) / (float)sensor->w;
			float jy = (2. * sampler.next_float()) / (float)sensor->h;

			for (int j = 0; j < v.size(); j++) {
				for (int i = 0; i < u.size(); i++) {
					Ray r = camera->generate_ray(u[i] + jx, v[j] + jy);
					SurfaceInteraction si;
					
					if (scene.intersect(r, si)) {

						if (!si.brdf)
							break;
						
						vec3 n = si.nor;
						vec3 t = glm::normalize(glm::cross(n, vec3(0., 1., 0.)));
						t = glm::normalize(glm::cross(t, n));
						vec3 b = glm::normalize(glm::cross(t, n));
						glm::mat3 tbn = glm::mat3(t, b, n);
						glm::mat3 inv_tbn = glm::transpose(tbn);

						vec3 wi = inv_tbn * (-r.d);
						
						for (int l = 0; l < scene.lights.size(); l++) {

							vec3 ld = scene.lights[l]->sample_light_direction();
							
							vec3 wo = inv_tbn * ld;

							//sensor->data[j * u.size() + i] = si.brdf->eval(wi, wo);
							sensor->set(i, j, si.brdf->eval(wi, wo));

						}

					}
				}
			}

		}
	protected:
		void link_params() {

		}
	};

	class AOIntegrator : public Integrator
	{
	public:
		AOIntegrator() : Integrator("AOIntegrator") {
			link_params();
		};

		void render(std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) {
			std::vector<Float> u = linspace<Float>(-1, 1, sensor->w);
			std::vector<Float> v = linspace<Float>(-1, 1, sensor->h);

			float jx = (2. * sampler.next_float()) / (float)sensor->w;
			float jy = (2. * sampler.next_float()) / (float)sensor->h;

			for (int j = 0; j < v.size(); j++) {
				for (int i = 0; i < u.size(); i++) {
					Ray r = camera->generate_ray(u[i] + jx, v[j] + jy);
					SurfaceInteraction si;

					if (scene.intersect(r, si)) {

						vec3 n = si.nor;
						vec3 t = glm::normalize(glm::cross(n, vec3(0., 1., 0.)));
						t = glm::normalize(glm::cross(t, n));
						vec3 b = glm::normalize(glm::cross(t, n));
						glm::mat3 tbn = glm::mat3(t, b, n);
						glm::mat3 inv_tbn = glm::transpose(tbn);

						Ray r_shadow;
						r_shadow.o = si.pos - 0.01f * r.d;
						
						int nSample = 1;
						Spectrum val(0.);
						for (int l = 0; l < nSample; l++) {

							vec3 wi = lt::square_to_uniform_hemisphere(sampler.next_float(), sampler.next_float());
							r_shadow.d = tbn * wi;

							SurfaceInteraction si_shadow;
							
							if (scene.intersect(r_shadow, si_shadow))
								val += Spectrum(1.) / (float(nSample));

						}

						sensor->set(i, j, val*0.5f);
						//sensor->set(i, j, r_shadow.o);

					}
				}
			}

		}
	protected:
		void link_params() {

		}
	};


}