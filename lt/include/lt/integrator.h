/**
 * @file integrator.h
 * @brief Defines classes related to rendering integrators.
 */

#pragma once
#include <lt/lt_common.h>
#include <lt/serialize.h>
#include <lt/scene.h>
#include <lt/camera.h>
#include <lt/sensor.h>
#include <lt/sampler.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/task.h>
#include <chrono>

namespace LT_NAMESPACE {


	class Integrator : public Serializable
	{
	public:
		
		/**
		 * @brief Constructor.
		 * @param type The type of the integrator.
		 */
		Integrator(const std::string& type) : Serializable(type) {
			n_sample = 1;
		};

		/**
		 * @brief Renders the scene.
		 * @param camera The camera used for rendering.
		 * @param sensor The sensor to capture the rendered image.
		 * @param scene The scene to render.
		 * @param sampler The sampler used for sampling.
		 * @return The time taken per pixel in milliseconds.
		 */
		float render(std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) {
			
			auto t1 = std::chrono::high_resolution_clock::now();

#if 0
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
#endif
#if 1
			int block_size = 16;
			#pragma omp parallel for collapse(2)
			for (int h = 0; h < sensor->h / block_size + 1; h++)
				for (int w = 0; w < sensor->w / block_size + 1; w++) {
					Sampler s;
					s.seed((h + w * (block_size + 1) +1 ) * n_sample);
					render_block(h,w, block_size,camera, sensor, scene, s);
				}
			
#endif
#if 0
			int block_size = 16;
			int h_num_block = sensor->h / block_size + 1;
			int w_num_block = sensor->w / block_size + 1;
			
			tbb::task_group tg;
			
			for (int h = 0; h < sensor->h / block_size + 1; h++)
				for (int w = 0; w < sensor->w / block_size + 1; w++) {
					tg.run([&] {
						Sampler s;// (sampler);
						s.seed(h + w * (block_size + 1));
						render_block(h, w, block_size, camera, sensor, scene, s);
					});
				}
			tg.wait();
			tbb::missing_wait();
#endif
			
			n_sample++;
			
			auto t2 = std::chrono::high_resolution_clock::now();
			float delta_time = (t2 - t1).count();
			// return ms per pixel
			return delta_time / (sensor->h * sensor->w);
		};

		/**
		 * @brief Renders a block of pixels in the scene.
		 * @param id_h The ID of the block in the vertical direction.
		 * @param id_w The ID of the block in the horizontal direction.
		 * @param block_size The size of the block.
		 * @param camera The camera used for rendering.
		 * @param sensor The sensor to capture the rendered image.
		 * @param scene The scene to render.
		 * @param sampler The sampler used for sampling.
		 */
		void render_block(uint32_t id_h, uint32_t id_w, uint32_t block_size, std::shared_ptr<Camera> camera, std::shared_ptr<Sensor> sensor, Scene& scene, Sampler& sampler) {
			uint32_t h_min = id_h * block_size;
			uint32_t w_min = id_w * block_size;
			
			uint32_t h_max = std::min((id_h + 1) * block_size, sensor->h);
			uint32_t w_max = std::min((id_w + 1) * block_size, sensor->w);
			for (int h = h_min; h < h_max; h++) {
				for (int w = w_min; w < w_max; w++) {

					float jw = (2. * sampler.next_float()) / (float)sensor->w;
					float jh = (2. * sampler.next_float()) / (float)sensor->h;

					Ray r = camera->generate_ray(sensor->u[w] + jw, sensor->v[h] + jh);
					Spectrum s = render_pixel(r, scene, sampler);

					sensor->add(w, h, s);

				}
			}
		}

		/**
		 * @brief Renders a single pixel in the scene.
		 * @param r The ray starting from the pixel.
		 * @param scene The scene to render.
		 * @param sampler The sampler used for sampling.
		 * @return The resulting contribution.
		 */
		virtual Spectrum render_pixel(Ray& r, Scene& scene, Sampler& sampler) = 0;
		
		/**
		 * @brief Estimates direct lighting contribution from random light source.
		 * @param r The ray representing the pixel.
		 * @param si Surface interaction data.
		 * @param scene The scene to render.
		 * @param sampler The sampler used for sampling.
		 * @return The estimated direct lighting contribution.
		 */
		Spectrum uniform_sample_one_light(Ray& r, SurfaceInteraction& si, Scene& scene, Sampler& sampler) {
			int n_light = scene.lights.size();

			if (n_light == 0)
				return Spectrum(0.);

			int light_idx = std::min((int)(sampler.next_float() * n_light), n_light - 1);
			Float light_pdf = (Float)(1.) / n_light;

			const std::shared_ptr<Light>& light = scene.lights[light_idx];

			return estimate_direct(r, si, light, scene, sampler) / light_pdf;
		}

		/**
		 * @brief Estimates direct lighting contribution from a light source.
		 * @param r The ray representing the pixel.
		 * @param si Surface interaction data.
		 * @param light The light source.
		 * @param scene The scene to render.
		 * @param sampler The sampler used for sampling.
		 * @return The estimated direct lighting contribution.
		 */
		Spectrum estimate_direct(Ray& r, SurfaceInteraction& si, const std::shared_ptr<Light>& light, Scene& scene, Sampler& sampler) {

			vec3 l = light->sample_light_direction();
			vec3 wo = si.to_local(-l);
			vec3 wi = si.to_local(-r.d);
			
			Ray rs(si.pos - r.d * 0.0001f, -l);
			if(!scene.intersect(rs))
				return si.brdf->eval(wi, wo);
			return Spectrum(0.);
		}

		uint32_t n_sample;
	
	};

	/**
	 * @brief Direct lighting integrator class.
	 */
	class DirectIntegrator : public Integrator
	{
	public:

		DirectIntegrator() : Integrator("DirectIntegrator") {
			link_params();
		};

		Spectrum render_pixel(Ray& r, Scene& scene, Sampler& sampler) {
			
			SurfaceInteraction si;
			
			Spectrum s(0.);

			if (scene.intersect(r, si)) {

				if (!si.brdf) {
					r = Ray(si.pos + r.d * 0.00001f, r.d);
					return render_pixel(r, scene, sampler);
				}
				
				s += uniform_sample_one_light(r, si, scene, sampler);
			}

			return s;
		}
	protected:
		void link_params() {

		}
	};

	/**
	 * @brief Path tracing integrator class.
	 */
	class PathIntegrator : public Integrator
	{
	public:
		PathIntegrator() : Integrator("PathIntegrator"),
			depth(32)
		{
			link_params();
		};


		Spectrum render_pixel(Ray& r, Scene& scene, Sampler& sampler) {

			Spectrum attenuation(1.);
			Spectrum s(0.);

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

			return s;
		}
		

		uint32_t depth; /**< Maximum depth of path tracing. */
	protected:
		void link_params() {
			
		}
	};

	
	/**
	 * @brief Ambient occlusion integrator class.
	 */
	class AOIntegrator : public Integrator
	{
	public:
		AOIntegrator() : Integrator("AOIntegrator") {
			link_params();
		};

		Spectrum render_pixel(Ray& r, Scene& scene, Sampler& sampler) {

			SurfaceInteraction si;
			
			Spectrum s(0.);

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

			return s;

		}
	protected:
		void link_params() {

		}
	};


}