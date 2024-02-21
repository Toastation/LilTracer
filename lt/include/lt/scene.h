#pragma once

#include <lt/lt_common.h>
#include <lt/geometry.h>
#include <lt/light.h>
#include <lt/brdf.h>



namespace LT_NAMESPACE {

class Scene
{
public:
	Scene() {
	}


	bool intersect(Ray r, SurfaceInteraction& si) {
		
		RTCRayHit rayhit;
		rayhit.ray.org_x = r.o.x; rayhit.ray.org_y = r.o.y; rayhit.ray.org_z = r.o.z;
		rayhit.ray.dir_x = r.d.x; rayhit.ray.dir_y = r.d.y; rayhit.ray.dir_z = r.d.z;
		rayhit.ray.tnear = 0.f;
		rayhit.ray.tfar = std::numeric_limits<float>::infinity();
		rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		rtcIntersect1(scene, &context, &rayhit);

		if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
			unsigned int geom_id = rayhit.hit.geomID;
			std::shared_ptr<Geometry> geom = geometries[geom_id];
			
			si.t = rayhit.ray.tfar;
			si.brdf = geom->brdf;
			si.pos = r.o + r.d * si.t;
			//si.nor = vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
			si.nor = geom->get_normal(rayhit, si.pos);

			return true;
		}

		return false;
	}

	void init_rtc() {
		device = rtcNewDevice(NULL);
		scene = rtcNewScene(device);

		for (int i = 0; i < geometries.size(); i++) {
			geometries[i]->init_rtc(device);
			rtcCommitGeometry(geometries[i]->rtc_geom);
			unsigned int geomID = rtcAttachGeometry(scene, geometries[i]->rtc_geom);
			rtcReleaseGeometry(geometries[i]->rtc_geom);
		}

		rtcCommitScene(scene);
	}

	RTCDevice device;
	RTCScene scene;

	std::vector<std::shared_ptr<Geometry>> geometries;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Brdf>> brdfs;
};



}
