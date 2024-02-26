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

			std::map<std::pair<uint32_t, uint32_t>, uint32_t> exist;
			for (int i = 0; i < fobj->face_count; i++) {
				
				glm::uvec3 idx;

				for (int j = 0; j < 3; j++) {


					int iv = fobj->indices[3 * i + j].p;
					int in = fobj->indices[3 * i + j].n;

					std::map<std::pair<uint32_t, uint32_t>, uint32_t>::iterator iter = exist.find({ iv,in });
					if (iter != exist.end()) {
						idx[j] = iter->second;
					}
					else {
						idx[j] = vertex.size();
						exist[{iv, in}] = vertex.size();
						vertex.push_back(vec3(fobj->positions[3 * iv], fobj->positions[3 * iv + 1], fobj->positions[3 * iv + 2]));
						normal.push_back(vec3(fobj->normals[3 * in], fobj->normals[3 * in + 1], fobj->normals[3 * in + 2]));
					}

				}

				triangle_indices.push_back(idx);

			}


			fast_obj_destroy(fobj);
		};

		void init_rtc(RTCDevice device) {
			rtc_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

			float* vb = (float*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), vertex.size());
			for (int i = 0; i < vertex.size(); i++) {
				vb[3 * i]     = vertex[i].x;
				vb[3 * i + 1] = vertex[i].y;
				vb[3 * i + 2] = vertex[i].z;
			}

			unsigned* ib = (unsigned*)rtcSetNewGeometryBuffer(rtc_geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned), triangle_indices.size());
			for (int i = 0; i < triangle_indices.size(); i++) {
				ib[3 * i]     = triangle_indices[i].x;
				ib[3 * i + 1] = triangle_indices[i].y;
				ib[3 * i + 2] = triangle_indices[i].z;
			}

		}

		vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos) {
			unsigned int prim_id = rayhit.hit.primID;
			glm::uvec3 face_nor = triangle_indices[prim_id];

			vec3 n1 = normal[face_nor.x];
			vec3 n2 = normal[face_nor.y];
			vec3 n3 = normal[face_nor.z];

			return n1 * rayhit.hit.u + n2 * rayhit.hit.v + n3 * (1 - rayhit.hit.u - rayhit.hit.v);

		}


		std::string filename;
		std::vector<vec3> normal;
		std::vector<vec3> vertex;
		std::vector<glm::uvec3> triangle_indices;


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


	protected:
		void link_params() {
			params.add("pos", Params::Type::VEC3 , &pos);
			params.add("rad", Params::Type::FLOAT, &rad);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

}
