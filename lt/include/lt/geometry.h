#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/brdf.h>
#include <fast_obj.h>
#include <embree3/rtcore.h>

namespace LT_NAMESPACE {

	class Geometry : public Serializable
	{
	public:
		Geometry(const std::string& type) : Serializable(type) {};

		virtual vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos) = 0;
		virtual void init_rtc(RTCDevice device) = 0;

		std::shared_ptr<Brdf> brdf;
		RTCGeometry rtc_geom;
	};


	class Mesh : public Geometry
	{
	public:
		Mesh() : Geometry("Mesh") {
			link_params();
		};

		void init() {
			// Parse obj
			fastObjMesh* fobj = fast_obj_read(filename.c_str());
			if (!fobj) {
				std::cerr << "Could not read : " << filename << std::endl;
				return;
			}

			vertex.resize(fobj->position_count);
			for (int i = 0; i < fobj->position_count; i++)
				vertex[i] = vec3(fobj->positions[3 * i], fobj->positions[3 * i + 1], fobj->positions[3 * i + 2]);

			normal.resize(fobj->normal_count);
			for (int i = 0; i < fobj->normal_count; i++)
				normal[i] = vec3(fobj->normals[3 * i], fobj->normals[3 * i + 1], fobj->normals[3 * i + 2]);


			faces_vertex.resize(fobj->face_count);
			faces_normal.resize(fobj->face_count);
			for (int i = 0; i < fobj->face_count; i++){
				faces_vertex[i] = glm::uvec3(fobj->indices[3 * i].p, fobj->indices[3 * i + 1].p, fobj->indices[3 * i + 2].p);
				faces_normal[i] = glm::uvec3(fobj->indices[3 * i].n, fobj->indices[3 * i + 1].n, fobj->indices[3 * i + 2].n);
			}

			fast_obj_destroy(fobj);
		};

		void init_rtc(RTCDevice device) {
			// Set embree
			rtc_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

			float* vb = (float*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), vertex.size());
			for (int i = 0; i < vertex.size(); i++) {
				vb[3 * i]     = vertex[i].x;
				vb[3 * i + 1] = vertex[i].y;
				vb[3 * i + 2] = vertex[i].z;
			}
			
			//float* no = (float*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), normal.size());
			//for (int i = 0; i < normal.size(); i++) {
			//	no[3 * i]     = normal[i].x;
			//	no[3 * i + 1] = normal[i].y;
			//	no[3 * i + 2] = normal[i].z;
			//}

			unsigned* ib = (unsigned*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned), faces_vertex.size());
			for (int i = 0; i < faces_vertex.size(); i++) {
				ib[3 * i]     = faces_vertex[i].x;
				ib[3 * i + 1] = faces_vertex[i].y;
				ib[3 * i + 2] = faces_vertex[i].z;
			}

		}

		vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos) {
			unsigned int prim_id = rayhit.hit.primID;
			glm::uvec3 face_nor = faces_normal[prim_id];

			vec3 n1 = normal[face_nor.x];
			vec3 n2 = normal[face_nor.y];
			vec3 n3 = normal[face_nor.z];

			return n1 * rayhit.hit.u + n2 * rayhit.hit.v + n3 * (1 - rayhit.hit.u - rayhit.hit.v);

		}


		std::string filename;
		std::vector<vec3> normal;
		std::vector<vec3> vertex;
		std::vector<glm::uvec3> faces_normal;
		std::vector<glm::uvec3> faces_vertex;


	protected:
		void link_params() {
			params.add("filename", Params::Type::PATH, &filename);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

	class Sphere : public Geometry
	{
	public:
		Sphere() :
			Geometry("Sphere"),
			pos(vec3(0.)),
			rad(1.)
		{
			link_params();

			this->brdf = std::shared_ptr<Brdf>(nullptr);
		};

		void init() {};

		void init_rtc(RTCDevice device) {
			// Set embree
			rtc_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_SPHERE_POINT);

			float* vb = (float*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, 4 * sizeof(float), 1);

			vb[0] = pos.x;
			vb[1] = pos.y;
			vb[2] = pos.z;
			vb[3] = rad;
		}

		vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos) {
			return (hit_pos - pos) / rad;
		}

		vec3 pos;
		float rad;

		//bool intersect(Ray r, SurfaceInteraction& si) {
		//	vec3 d = r.o - pos;
		//	float b = glm::dot(d, r.d);
		//	float c = glm::dot(d, d) - rad*rad;
		//	float t = b * b - c;
		//	if (t > 0.) {
		//		t = -b - std::sqrt(t);
		//		si.t = t;
		//		si.pos = r.o + t * r.d;
		//		si.nor = (si.pos - pos) / rad;
		//		si.brdf = brdf;
		//		return true;
		//	}
		//	return false;
		//}

	protected:
		void link_params() {
			params.add("pos", Params::Type::VEC3 , &pos);
			params.add("rad", Params::Type::FLOAT, &rad);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

}
