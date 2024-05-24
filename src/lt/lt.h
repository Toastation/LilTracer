#pragma once

#include <lt/brdf_common.h>
#include <lt/camera.h>
#include <lt/geometry.h>
#include <lt/integrator.h>
#include <lt/io.h>
#include <lt/io_exr.h>
#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/sampler.h>
#include <lt/scene.h>
#include <lt/sensor.h>

namespace LT_NAMESPACE {

	inline bool cornell_box(Scene& scn, Renderer& ren)
	{
		return generate_from_json("", R"(
	    	{
	 		"integrator": {
	 			"type":"DirectIntegrator"
	 		},
	 		"brdf": [
	 			{"type":"Diffuse","name":"diff","albedo":[0.2,0.5,0.8]}
	 		],
			"background" : {
				"type":"EnvironmentLight",
				"texture":"kloofendal_48d_partly_cloudy_puresky_1k.exr",
				"intensity" : 1.0
			},
	 		"nlight": [
	 			{"type":"DirectionnalLight", "intensity" : 1.0, "dir" : [-0.0, -1.0, -1.0] },
	 			{"type":"DirectionnalLight", "intensity" : 1.0, "dir" : [-1.0, -1.0, -1.0] }
	 		],
	 		"geometries": [
	 			{"type":"Mesh","brdf":"diff", "filename" : "../../../data/cornell.obj"}
	 		],
	 		"sensor": {
	 			"width" : 1080,
	 			"height" : 720
	 		},
	 		"camera": {
	 			"type":"PerspectiveCamera",
	 			"fov" : 40,
	 			"aspect" : 1.5,
	 			"center" : [0.0,1.0,0.0],
	 			"pos" : [0.0,1.0,4.0]
	 		}
	 	}
	 	)",
			scn, ren);
	}

	inline bool dir_light(Scene& scn, Renderer& ren)
	{
		return generate_from_json("", R"(
	   	{
			"integrator": {
				"type":"DirectIntegrator"
			},
			"brdf": [
				{"type":"Diffuse","name":"sphere_brdf","albedo":[0.2,0.5,0.8]}
			],
			"light": [
				{"type":"DirectionnalLight", "intensity" : 1.0, "dir" : [1.0, 0.0, 0.0] }
			],
			"geometries": [
				{"type":"Sphere","brdf":"sphere_brdf" , "pos":[0,0,0], "rad":1.0}
			],
			"sensor": {
				"type" : "Sensor",
				"width" : 512,
				"height" : 512
			},
			"camera": {
				"type":"PerspectiveCamera",
				"fov" : 30,
				"aspect" : 1.0,
				"center" : [0.0,0.0,0.0],
				"pos" : [5.0,0.0,0.0]
			}
		}
		)",
			scn, ren);
	}

} // namespace LT_NAMESPACE