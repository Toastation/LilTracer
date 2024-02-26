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


	/*bool intersect16(RTCRayHit16 rhs, SurfaceInteraction* si, int* valid) {

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		rtcIntersect16(valid,scene, & context, &rhs);

		for (int i = 0; i < 16; i++) {
			if (rhs.hit.geomID[i] != RTC_INVALID_GEOMETRY_ID) {
				valid[i] = -1;

				std::shared_ptr<Geometry> geom = geometries[rhs.hit.geomID[i]];

				si[i].t = rhs.ray.tfar[i];
				si[i].brdf = geom->brdf;
				si[i].pos = vec3(rhs.ray.org_x[i] + si[i].t * rhs.ray.dir_x[i], rhs.ray.org_y[i] + si[i].t * rhs.ray.dir_y[i], rhs.ray.org_z[i] + si[i].t * rhs.ray.dir_z[i]);
				si[i].nor = geom->get_normal(rtcGetRayHitFromRayHitN((RTCRayHitN*)&rhs,16,i), si[i].pos);
			}
			else {
				valid[i] = 0;
			}
		}

	}*/

	bool intersect(const Ray& r, SurfaceInteraction& si) {
		
		RTCRayHit rayhit;
		rayhit.ray.org_x = r.o.x; rayhit.ray.org_y = r.o.y; rayhit.ray.org_z = r.o.z;
		rayhit.ray.dir_x = r.d.x; rayhit.ray.dir_y = r.d.y; rayhit.ray.dir_z = r.d.z;
		rayhit.ray.tnear = 0.f;
		rayhit.ray.tfar = std::numeric_limits<float>::infinity();
		rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

		rtcIntersect1(scene, &context, &rayhit);

		if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
			unsigned int geom_id = rayhit.hit.geomID;
			std::shared_ptr<Geometry> geom = geometries[geom_id];
			
			si.t = rayhit.ray.tfar;
			si.brdf = geom->brdf;
			si.pos = r.o + r.d * si.t;
			si.nor = geom->get_normal(rayhit, si.pos);
			//si.nor = vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);

			si.finalize();
			return true;
		}

		return false;
	}

	bool intersect(const Ray& r) {
		
		RTCRay ray;
		ray.org_x = r.o.x; ray.org_y = r.o.y; ray.org_z = r.o.z;
		ray.dir_x = r.d.x; ray.dir_y = r.d.y; ray.dir_z = r.d.z;
		ray.tnear = 0.f;
		ray.tfar = std::numeric_limits<float>::infinity();
		ray.flags = 0;

		rtcOccluded1(scene, &context, &ray);

		if (ray.tfar < 0.) {
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


		rtcInitIntersectContext(&context);
	}

	RTCDevice device;
	RTCScene scene;
	RTCIntersectContext context;

	std::vector<std::shared_ptr<Geometry>> geometries;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Brdf>> brdfs;
};



}
