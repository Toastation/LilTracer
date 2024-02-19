#pragma once
#include <lt/lt_common.h>
#include <lt/scene.h>
#include <lt/renderer.h>

#include <nlohmann/json.hpp>

namespace LT_NAMESPACE {

	using json = nlohmann::json;


	static void json_set_float(const json& j, float* ptr) {
		*ptr = j;
	}

	static void json_set_vec3(const json& j, vec3* ptr) {
		(*ptr)[0] = j[0];
		(*ptr)[1] = j[1];
		(*ptr)[2] = j[2];
	}

	static void json_set_path(const json& j, std::string* ptr) {
		*ptr = std::string(j);
	}

	static void json_set_brdf(const json& j, std::shared_ptr<Brdf>* ptr, std::map<std::string, std::shared_ptr<Brdf>>& ref) {
		std::string brdf_name = j;
		*ptr = ref[brdf_name];
	}

	static void set_params(const json& j, const Params& params, std::map<std::string, std::shared_ptr<Brdf>>& brdf_ref) {

		for (int i = 0; i < params.count; i++) {

			if (j.contains(params.names[i])) {

				switch (params.types[i])
				{
				case Params::Type::FLOAT:
					json_set_float(j[params.names[i]], (float*)params.ptrs[i]);
					break;
				case Params::Type::VEC3:
					json_set_vec3(j[params.names[i]], (vec3*)params.ptrs[i]);
					break;
				case Params::Type::PATH:
					json_set_path(j[params.names[i]], (std::string*)params.ptrs[i]);
					break;
				case Params::Type::BRDF:
					json_set_brdf(j[params.names[i]], (std::shared_ptr<Brdf>*)params.ptrs[i], brdf_ref);
					break;
				default:
					std::cerr << "json to Params::Type not defined" << std::endl;
					break;
				}
			}
			else {
				std::cout << "missing : " << params.names[i] << std::endl;
			}
		}

	}

	static bool generate_from_json(const std::string& str, Scene& scn, Renderer& ren) {
		std::map<std::string, std::shared_ptr<Brdf>> brdf_ref;

		json json_scn = json::parse(str);

		ren.sampler = std::make_shared<Sampler>();

		// Parse Integrator
		if (json_scn.contains("integrator")) {
			json json_integrator = json_scn["integrator"];
			std::shared_ptr<Integrator> integrator = Factory<Integrator>::create(json_integrator["type"]);

			if (!integrator)
				return false;

			set_params(json_integrator, integrator->params, brdf_ref);
			integrator->init();

			ren.integrator = integrator;
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing integrator" << std::endl;
		}


		// Parse Sensor
		if (json_scn.contains("sensor")) {
			json json_sensor = json_scn["sensor"];
			uint16_t width = json_sensor["width"];
			uint16_t height = json_sensor["height"];
			ren.sensor = std::make_shared<Sensor>(width, height);
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing sensor" << std::endl;
		}

		// Parse Camera
		if (json_scn.contains("camera")) {
			json json_camera = json_scn["camera"];
			std::shared_ptr<Camera> camera = Factory<Camera>::create(json_camera["type"]);

			if (!camera)
				return false;

			set_params(json_camera, camera->params, brdf_ref);
			camera->init();

			ren.camera = camera;
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing camera" << std::endl;
		}

		// Parse BRDF
		if (json_scn.contains("brdf")) {
			for (const auto& json_brdf : json_scn["brdf"]) {
				std::shared_ptr<Brdf> brdf = Factory<Brdf>::create(json_brdf["type"]);
				if (!brdf)
					return false;

				brdf_ref[json_brdf["name"]] = brdf;

				set_params(json_brdf, brdf->params, brdf_ref);
				brdf->init();

				scn.brdfs.push_back(brdf);
			}
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing brdf" << std::endl;
		}

		// Parse Light
		if (json_scn.contains("light")) {
			for (const auto& json_light : json_scn["light"]) {

				std::shared_ptr<Light> light = Factory<Light>::create(json_light["type"]);
				if (!light)
					return false;

				set_params(json_light, light->params, brdf_ref);
				light->init();

				scn.lights.push_back(light);
			}
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing light" << std::endl;
		}

		// Parse Shapes
		if (json_scn.contains("shapes")) {
			for (const auto& json_shape : json_scn["shapes"]) {
				std::cout << json_shape << std::endl;

				std::shared_ptr<Shape> shape = Factory<Shape>::create(json_shape["type"]);
				if (!shape)
					return false;

				set_params(json_shape, shape->params, brdf_ref);
				shape->init();

				scn.shapes.push_back(shape);

			}
		}
		else {
			std::cerr << "Abort generate_from_json, cause : Missing shapes" << std::endl;
		}

		return true;

	}

	inline bool cornell_box(Scene& scn, Renderer& ren) {
		return generate_from_json(R"(
	   	{
			"integrator": {
				"type":"AOIntegrator"
			},
			"brdf": [
				{"type":"Diffuse","name":"diff","albedo":[0.2,0.5,0.8]},
				{"type":"RoughConductor","name":"rough","albedo":[0.2,0.5,0.8],"alpha_x":0.5,"alpha_x":0.15}
			],
			"light": [
				{"type":"DirectionnalLight", "intensity" : 1.25, "dir" : [1.0, 0.0, 0.0] },
				{"type":"DirectionnalLight", "intensity" : 1.98, "dir" : [0.0, 1.0, 0.0] }
			],
			"shapes": [
				{"type":"Sphere","brdf":"diff" , "pos":[0,0.5,0], "rad":1.0},
				{"type":"Sphere","brdf":"diff" , "pos":[0,-0.5,0], "rad":1.0},
				{"type":"Mesh","brdf":"rough", "filename" : "../file.obj"}
			],
			"sensor": {
				"width" : 1080,
				"height" : 720
			},
			"camera": {
				"type":"PerspectiveCamera",
				"fov" : 30,
				"aspect" : 1.5,
				"center" : [0,0,0],
				"pos" : [-2.0,0.0,0.0]
			}
		}
		)", scn, ren);
	}


}