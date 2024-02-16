#pragma once

#include <lt/lt_common.h>
#include <lt/mesh.h>
#include <lt/light.h>
#include <lt/ray.h>
#include <nlohmann/json.hpp>

namespace LT_NAMESPACE {

using json = nlohmann::json;

class Scene
{
public:
	Scene();
	~Scene();

	bool intersect(const Ray& r, SurfaceInteraction& si) {

		SurfaceInteraction si_t;
		
		bool has_intersect = false;

		for (int i = 0; i < shapes.size(); i++) {
			if (shapes[i]->intersect(r, si_t) && si_t.t < si.t) {
				si = si_t;
				has_intersect = true;
			}
		}

		return has_intersect;
		
	}

	static void json_set_float(const json& j, float* ptr) {
		*ptr = j;
	}
	
	static void json_set_vec3(const json & j, vec3* ptr) {
		(*ptr)[0] = j[0];
		(*ptr)[1] = j[1];
		(*ptr)[2] = j[2];
	}

	static void set_params(const json& j, const Params& params) {

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

	static Scene* generate_from_json(const std::string& str) {
		Scene* scn = new Scene();
		
		json json_scn = json::parse(str);

		std::cout << json_scn << std::endl;

		// Parse BRDF
		if (json_scn.contains("brdf")) {
			for (const auto& json_brdf : json_scn["brdf"]) {
				std::cout << json_brdf << std::endl;
			
				std::shared_ptr<Brdf> brdf = Factory<Brdf>::create(json_brdf["type"]);
				if (!brdf)
					break;

				set_params(json_brdf, brdf->params);

				scn->brdfs.push_back(brdf);
			}
		}

		// Parse Light
		if (json_scn.contains("light")) {
			for (const auto& json_light : json_scn["light"]) {
				std::cout << json_light << std::endl;

				std::shared_ptr<Light> light = Factory<Light>::create(json_light["type"]);
				if (!light)
					break;

				set_params(json_light, light->params);

				scn->lights.push_back(light);
			}
		}

		// Parse Shapes
		for (const auto& shape : json_scn["shape"]) {
			std::cout << shape << std::endl;
		}



		return scn;

	}

	std::vector<std::shared_ptr<Shape>> shapes;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Brdf>> brdfs;
};

inline Scene* cornell_box() {
	return Scene::generate_from_json(R"(
	   	{
			"brdf": [
				{"type":"Diffuse","name":"bsdf1","albedo":[0.2,0.5,0.8]},
				{"type":"RoughConductor","name":"bsdf2","albedo":[0.2,0.5,0.8],"alpha_x":0.5,"alpha_x":0.15}
			],
			"light": [
				{"type":"DirectionnalLight", "intensity" : 1.25, "dir" : [1.0, 0.0, 0.0] },
				{ "type":"DirectionnalLight","intensity" : 1.98, "dir" : [0.0, 1.0, 0.0] }
			],
			"shapes": [
				{"type":"mesh","name":"shape1"},
				{"type":"mesh","name":"shape2"}
			]
		}
		)");
}


/*class CornellBox : public Scene
{
public:
	CornellBox() {

		json scn = json::parse(R"(
	   	{
			"brdf": [
				{"type":"Diffuse","name":"bsdf1","albedo":[0.2,0.5,0.8]},
				{"type":"RoughConductor","name":"bsdf2","albedo":[0.2,0.5,0.8]}
			],
			"shapes": [
				{"type":"mesh","name":"shape1"},
				{"type":"mesh","name":"shape2"}
			]
		}
		)");



		return;
	};

private:

};*/


//inline Scene cornell_box() {
//
//	Scene scn;
//
//	scn.objs.push_back(new Sphere(vec3(0.), 2.));
//	scn.objs.push_back(new Sphere(vec3(0.,4.,0.), 2.));
//	scn.objs.push_back(new Sphere(vec3(0.,0.,3.), 1.));
//
//	return scn;
//
//}


}
