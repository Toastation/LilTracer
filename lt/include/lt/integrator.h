#pragma once
#include <lt/lt_common.h>
#include <lt/serialize.h>
#include <lt/scene.h>
#include <lt/camera.h>
#include <lt/sensor.h>
#include <lt/sampler.h>

#include <chrono>

namespace LT_NAMESPACE {

	class Integrator : public Serializable
	{
	public:
		
		Integrator(const std::string& type) : Serializable(type) {};

		virtual float render(std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) {
			
			auto t1 = std::chrono::high_resolution_clock::now();
			for (int h = 0; h < sensor->h; h++) {
				for (int w = 0; w < sensor->w; w++) {

					float jw = (2. * sampler.next_float()) / (float)sensor->w;
					float jh = (2. * sampler.next_float()) / (float)sensor->h;

					Ray r = camera->generate_ray(sensor->u[w] + jw, sensor->v[h] + jh);
					Spectrum s;
					render_pixel(r, s, scene, sampler);
						
					sensor->add(w, h, s);					
				}
			}
			auto t2 = std::chrono::high_resolution_clock::now();

			float delta_time = (t2 - t1).count();
			// return ms per pixel
			return delta_time / (sensor->h * sensor->w);
		};
		virtual void render_pixel(Ray& r, Spectrum& s, Scene& scene, Sampler& sampler) = 0;
		
		Spectrum uniform_sample_one_light(Ray& r, SurfaceInteraction& si, Scene& scene, Sampler& sampler) {
			int n_light = scene.lights.size();

			if (n_light == 0)
				return Spectrum(0.);

			int light_idx = std::min((int)(sampler.next_float() * n_light), n_light - 1);
			Float light_pdf = (Float)(1.) / n_light;

			const std::shared_ptr<Light>& light = scene.lights[light_idx];

			return estimate_direct(r, si, light, scene, sampler) / light_pdf;
		}

		Spectrum estimate_direct(Ray& r, SurfaceInteraction& si, const std::shared_ptr<Light>& light, Scene& scene, Sampler& sampler) {

			vec3 l = light->sample_light_direction();
			vec3 wo = si.to_local(-l);
			vec3 wi = si.to_local(-r.d);
			
			Ray rs(si.pos - r.d * 0.0001f, -l);
			if(!scene.intersect(rs))
				return si.brdf->eval(wi, wo);
			return Spectrum(0.);
		}
	
	};


	class DirectIntegrator : public Integrator
	{
	public:
		DirectIntegrator() : Integrator("DirectIntegrator") {
			link_params();
		};

		void render_pixel(Ray& r, Spectrum& s, Scene& scene, Sampler& sampler) {
			
			SurfaceInteraction si;
			
			s = Spectrum(0.);

			if (scene.intersect(r, si)) {

				if (!si.brdf) {
					r = Ray(si.pos + r.d * 0.00001f, r.d);
					render_pixel(r, s, scene, sampler);
					return;
				}
				
				s += uniform_sample_one_light(r, si, scene, sampler);
			}
		}
	protected:
		void link_params() {

		}
	};

	class PathIntegrator : public Integrator
	{
	public:
		PathIntegrator() : Integrator("PathIntegrator"),
			depth(32)
		{
			link_params();
		};


		void render_pixel(Ray& r, Spectrum& s, Scene& scene, Sampler& sampler) {

			Spectrum attenuation(1.);
			s = Spectrum(0.);

			for (int d = 0; d < depth; d++) {

				SurfaceInteraction si;
				if (scene.intersect(r, si)) {

					if (!si.brdf) {
						r = Ray(si.pos + r.d * 0.00001f, r.d);
						d--;
						continue;
					}
					
					vec3 p = si.pos - r.d * 0.00001f;

					// Compute Light contrib
					s += attenuation * uniform_sample_one_light(r,si,scene,sampler);
					
					// Compute BRDF  contrib
					vec3 wi = si.to_local(-r.d);
					vec3 wo = si.brdf->sample(wi, sampler);
					Float wo_pdf = si.brdf->pdf(wo);
					Spectrum brdf_cos_weighted = si.brdf->eval(wi, wo);
					attenuation *= brdf_cos_weighted / wo_pdf;

					r = Ray(p, si.to_world(wo) );
				}
				else 
				{
					float a = 0.5 * (r.d.y + 1.0);
					Spectrum bg_color = ((1.0f - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0));
					s += attenuation * bg_color ;
					break;
				}
			}

		}
		

		uint32_t depth;
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

		void render_pixel(Ray& r, Spectrum& s, Scene& scene, Sampler& sampler) {

			SurfaceInteraction si;
			
			s = Spectrum(0.);

			if (scene.intersect(r, si)) {

				Ray rs;
				rs.o = si.pos - 0.001f * r.d;
						
				int nSample = 1;
				
				for (int l = 0; l < nSample; l++) {

					vec3 wi = lt::square_to_uniform_hemisphere(sampler.next_float(), sampler.next_float());

					rs.d = si.to_world(wi);

					if (!scene.intersect(rs))
						s += Spectrum(1.) / (float(nSample));

				}
			
			}

		}
	protected:
		void link_params() {

		}
	};


}