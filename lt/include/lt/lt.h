#pragma once

#include <lt/brdf.h>
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

inline bool vis_mis_scn(Scene& scn, Renderer& ren)
{
    return generate_from_json(R"(
	{
		"integrator": {
			"type":"DirectIntegrator"
		},
		"brdf": [
			{"type":"GGXMicrosurface","name":"plane1","roughx":0.5,"roughy":0.5},
			{"type":"GGXMicrosurface","name":"plane2","roughx":0.1,"roughy":0.1},
			{"type":"GGXMicrosurface","name":"plane3","roughx":0.01,"roughy":0.01},
			{"type":"GGXMicrosurface","name":"plane4","roughx":0.001,"roughy":0.001},
			{"type":"Diffuse","name":"floor","albedo":[0.3,0.3,0.3]},
			{"type":"Emissive","name":"sphere_brdf","intensity":[100.0,100.0,100.0]}
		],
		"geometries": [
			{"type":"Mesh","brdf":"plane1", "filename" : "../../../data/vis_mis/plane1.obj"},
			{"type":"Mesh","brdf":"plane2", "filename" : "../../../data/vis_mis/plane2.obj"},
			{"type":"Mesh","brdf":"plane3", "filename" : "../../../data/vis_mis/plane3.obj"},
			{"type":"Mesh","brdf":"plane4", "filename" : "../../../data/vis_mis/plane4.obj"},
			{"type":"Mesh","brdf":"floor", "filename" : "../../../data/vis_mis/studio.obj"},
			{"type":"Sphere","brdf":"sphere_brdf", "pos":[ 0.4,0.7,0.5], "rad" : 0.015221},
			{"type":"Sphere","brdf":"sphere_brdf", "pos":[ 0.0,0.7,0.5], "rad" : 0.075804},
			{"type":"Sphere","brdf":"sphere_brdf", "pos":[-0.4,0.7,0.5], "rad" : 0.144387}
		],
		"sensor": {
			"width" : 1080,
			"height" : 720
		},
		"camera": {
			"type":"PerspectiveCamera",
			"fov" : 25,
			"aspect" : 1.5,
			"center" : [0.0,0.481935,-0.331243],
			"pos" : [0.0,1.12458,-2.59721]
		}
	}
	)",
        scn, ren);
}

inline bool prob_scn(Scene& scn, Renderer& ren)
{
    return generate_from_json(R"(
	   	{
			"integrator": {
				"type":"BrdfIntegrator"
			},
			"brdf": [
				{"type":"Diffuse","name":"base","albedo":[0.2,0.2,0.2]},
				{"type":"Diffuse","name":"floor","albedo":[0.4,0.4,0.4]},
				{"type":"Diffuse","name":"mat","albedo":[0.5,0.4,0.3]}
			],
			"background" : {
				"type":"EnvironmentLight",
				"texture":"kloofendal_48d_partly_cloudy_puresky_1k.exr",
				"intensity" : 1.0
			},
			"nlight": [
				{"type":"DirectionnalLight", "intensity" : 1.0, "dir" : [0.0, -1.0, 1.0] },
				{"type":"DirectionnalLight", "intensity" : 1.0, "dir" : [1.0, -1.0, 1.0] }
			],
			"geometries": [
				{"type":"Mesh","brdf":"base", "filename" : "../../../data/lte-orb/base.obj"},
				{"type":"Mesh","brdf":"mat", "filename" : "../../../data/lte-orb/equation.obj"},
				{"type":"Mesh","brdf":"base", "filename" : "../../../data/lte-orb/inner_sphere.obj"},
				{"type":"Mesh","brdf":"mat", "filename" : "../../../data/lte-orb/outer_sphere.obj"},
				{"type":"Mesh","brdf":"floor", "filename" : "../../../data/lte-orb/studio.obj"}
			],
			"sensor": {
				"width" : 1080,
				"height" : 720
			},
			"camera": {
				"type":"PerspectiveCamera",
				"fov" : 40,
				"aspect" : 1.5,
				"center" : [0.0,0.1,0.0],
				"pos" : [-0.118722,0.415848,-0.491901]
			}
		}
		)",
        scn, ren);
}

inline bool cornell_box(Scene& scn, Renderer& ren)
{
    return generate_from_json(R"(
	    	{
	 		"integrator": {
	 			"type":"PathIntegrator"
	 		},
	 		"brdf": [
	 			{"type":"Diffuse","name":"diff","albedo":[0.2,0.5,0.8]},
	 			{"type":"RoughConductor","name":"rough","albedo":[0.2,0.5,0.8],"alpha_x":0.5,"alpha_x":0.15}
	 		],
	 		"light": [
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
    return generate_from_json(R"(
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