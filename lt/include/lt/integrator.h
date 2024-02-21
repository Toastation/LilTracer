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
			for (int j = 0; j < sensor->h; j++) {
				for (int i = 0; i < sensor->w; i++) {

					float jx = (2. * sampler.next_float()) / (float)sensor->w;
					float jy = (2. * sampler.next_float()) / (float)sensor->h;

					Ray r = camera->generate_ray(sensor->u[i] + jx, sensor->v[j] + jy);
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
					
						vec3 rad = Spectrum(0.);
						for (int l = 0; l < scene.lights.size(); l++) {

							vec3 ld = scene.lights[l]->sample_light_direction();
							
							vec3 wo = inv_tbn * ld;

							rad += si.brdf->eval(wi, wo);
							//sensor->data[j * u.size() + i] = si.brdf->eval(wi, wo);
							

						}
						sensor->set(i, j, rad);

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

			for (int j = 0; j < sensor->h; j++) {
				for (int i = 0; i < sensor->w; i++) {

					float jx = (2. * sampler.next_float()) / (float)sensor->w;
					float jy = (2. * sampler.next_float()) / (float)sensor->h;

					Ray r = camera->generate_ray(sensor->u[i] + jx, sensor->v[j] + jy);
					SurfaceInteraction si;

					if (scene.intersect(r, si)) {

						vec3 n = si.nor;
						vec3 t, b;
						
						if (n.z < -0.999999)
						{
							t = vec3(0, -1, 0);
							b = vec3(-1, 0, 0);
						}
						else
						{
							float c1 = 1. / (1. + n.z);
							float c2 = -n.x * n.y * c1;
							t = glm::normalize(vec3(1.f - n.x * n.x * c1, c2, -n.x));
							b = glm::normalize(vec3(c2, 1.f - n.y * n.y * c1, -n.y));
						}
						

						glm::mat3 tbn = glm::mat3(t, b, n);
						glm::mat3 inv_tbn = glm::transpose(tbn);

						Ray r_shadow;
						r_shadow.o = si.pos - 0.001f * r.d;
						
						int nSample = 1;
						Spectrum val(0.);
						for (int l = 0; l < nSample; l++) {

							vec3 wi = lt::square_to_uniform_hemisphere(sampler.next_float(), sampler.next_float());

							// Transform wi from local frame to world space.
							r_shadow.d = vec3(t.x * wi.x + b.x * wi.y + n.x * wi.z,
								t.y * wi.x + b.y * wi.y + n.y * wi.z,
								t.z * wi.x + b.z * wi.y + n.z * wi.z);

							//r_shadow.d = tbn * wi;

							SurfaceInteraction si_shadow;
							
							if (!scene.intersect(r_shadow, si_shadow))
								val += Spectrum(1.) / (float(nSample));

						}

						//sensor->set(i, j, glm::abs(si.nor));
						//sensor->set(i, j, (si.pos));
						sensor->set(i, j, val);
						//sensor->set(i, j, vec3(si.t)*0.1f);

					}
				}
			}

		}
	protected:
		void link_params() {

		}
	};


}