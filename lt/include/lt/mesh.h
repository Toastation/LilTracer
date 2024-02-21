#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/brdf.h>
#include <fast_obj.h>
#include <embree3/rtcore.h>

namespace LT_NAMESPACE {

	class Shape : public Serializable
	{
	public:
		Shape(const std::string& type) : Serializable(type) {};

		virtual bool intersect(Ray r, SurfaceInteraction& si) = 0;
		std::shared_ptr<Brdf> brdf;
	};


	class Mesh : public Shape
	{
	public:
		Mesh() : Shape("Mesh") {
			link_params();
		};

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
				si.t = rayhit.ray.tfar;
				si.brdf = brdf;

				unsigned int id = rayhit.hit.primID;

				glm::uvec3 face_vtx = faces_vertex[id];
				glm::uvec3 face_nor = faces_normal[id];

				vec3 v1 = vertex[face_vtx.x];
				vec3 v2 = vertex[face_vtx.y];
				vec3 v3 = vertex[face_vtx.z];

				vec3 n1 = normal[face_nor.x];
				vec3 n2 = normal[face_nor.y];
				vec3 n3 = normal[face_nor.z];

				si.pos = r.o + r.d * si.t; 
				//si.pos = v1* rayhit.hit.u + v3 * rayhit.hit.v + v2 * (1 - rayhit.hit.u - rayhit.hit.v);
				si.nor = n1 * rayhit.hit.u + n2 * rayhit.hit.v + n3 * (1 - rayhit.hit.u - rayhit.hit.v);

				return true;
			}
			else {
				si.t = std::numeric_limits<Float>::infinity();
			}

			return false;
		}

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


			// Set embree
			device = rtcNewDevice(NULL);
			scene = rtcNewScene(device);
			geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

			float* vb = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), fobj->position_count);
			
			for (int i = 0; i < 3*fobj->position_count; i++) {
				vb[i] = fobj->positions[i];
			}

			unsigned* ib = (unsigned*)rtcSetNewGeometryBuffer(geom,	RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned), fobj->face_count);

			for (int i = 0; i < fobj->index_count; i++) {
				ib[i] = fobj->indices[i].p;
			}


			rtcCommitGeometry(geom);
			rtcAttachGeometry(scene, geom);
			rtcReleaseGeometry(geom);
			rtcCommitScene(scene);


			fast_obj_destroy(fobj);
		};

		std::string filename;
		std::vector<vec3> normal;
		std::vector<vec3> vertex;
		std::vector<glm::uvec3> faces_normal;
		std::vector<glm::uvec3> faces_vertex;

		RTCDevice device;
		RTCScene scene;
		RTCGeometry geom;

	protected:
		void link_params() {
			params.add("filename", Params::Type::PATH, &filename);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

	class Sphere : public Shape
	{
	public:
		Sphere() :
			Shape("Sphere"),
			pos(vec3(0.)),
			rad(1.)
		{
			link_params();

			this->brdf = std::shared_ptr<Brdf>(nullptr);
		};

		bool intersect(Ray r, SurfaceInteraction& si) {
			vec3 d = r.o - pos;
			float b = glm::dot(d, r.d);
			float c = glm::dot(d, d) - rad*rad;
			float t = b * b - c;
			if (t > 0.) {
				t = -b - std::sqrt(t);
				si.t = t;
				si.pos = r.o + t * r.d;
				si.nor = (si.pos - pos) / rad;
				si.brdf = brdf;
				return true;
			}
			return false;
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
