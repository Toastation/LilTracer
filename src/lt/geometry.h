/**
 * @file
 * @brief Definitions of the Geometry, Mesh, and Sphere classes.
 */

#pragma once

#include <embree3/rtcore.h>
#include <fast_obj/fast_obj.h>
#include <lt/brdf_common.h>
#include <lt/lt_common.h>
#include <lt/ray.h>

namespace LT_NAMESPACE {

/**
 * @brief Abstract base class for geometric objects in the scene.
 */
class Geometry : public Serializable {
public:
    /**
     * @brief Constructor for Geometry.
     * @param type The type of geometry.
     */
    Geometry(const std::string& type)
        : Serializable(type) 
    {
        local_to_world = glm::mat4(1.);
        local_to_world[0][0] = 1;
        local_to_world[1][1] = 1;
        local_to_world[2][2] = 1;
    };

    /**
     * @brief Pure virtual function for computing the normal at a hit position.
     * @param rayhit Information about the ray hit.
     * @param hit_pos The position of the hit.
     * @return The normal vector at the hit position.
     */
    virtual vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos) = 0;

    /**
     * @brief Pure virtual function for initializing Embree RTC geometry.
     * @param device The Embree RTC device.
     */
    virtual void init_rtc(RTCDevice device) = 0;

    std::shared_ptr<Brdf>
        brdf; /**< Pointer to the BRDF associated with the geometry. */

    RTCGeometry rtc_geom; /**< Embree RTC geometry. */
    int rtc_id;
    glm::mat4 local_to_world;
};

/**
 * @brief Class representing a triangle mesh geometry.
 */
class Mesh : public Geometry {
public:
    /**
     * @brief Default constructor for Mesh.
     * Initializes the geometry type and links parameters.
     */
    Mesh()
        : Geometry("Mesh")
    {
        link_params();
    };

    /**
     * @brief Initialize the mesh geometry by parsing an OBJ file.
     */
    void init()
    {
        // Parse obj
        fastObjMesh* fobj = fast_obj_read(filename.c_str());
        if (!fobj) {
            Log(logError) << "Could not read : " << filename;
            return;
        }

        std::map<std::pair<uint32_t, uint32_t>, uint32_t> exist;
        for (int i = 0; i < fobj->face_count; i++) {
            glm::uvec3 idx;

            for (int j = 0; j < 3; j++) {
                int iv = fobj->indices[3 * i + j].p;
                int in = fobj->indices[3 * i + j].n;

                std::map<std::pair<uint32_t, uint32_t>, uint32_t>::iterator iter = exist.find({ iv, in });
                if (iter != exist.end()) {
                    idx[j] = iter->second;
                } else {
                    idx[j] = vertex.size();
                    exist[{ iv, in }] = vertex.size();
                    vertex.push_back(vec3(fobj->positions[3 * iv],
                        fobj->positions[3 * iv + 1],
                        fobj->positions[3 * iv + 2]));
                    normal.push_back(vec3(fobj->normals[3 * in],
                        fobj->normals[3 * in + 1],
                        fobj->normals[3 * in + 2]));
                }
            }

            triangle_indices.push_back(idx);
        }

        fast_obj_destroy(fobj);
    };

    /**
     * @brief Initialize the Embree RTC geometry for the mesh.
     * @param device The Embree RTC device.
     */
    void init_rtc(RTCDevice device)
    {
        rtc_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        float* vb = (float*)rtcSetNewGeometryBuffer(
            rtc_geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
            3 * sizeof(float), vertex.size());
        for (int i = 0; i < vertex.size(); i++) {
            vb[3 * i] = vertex[i].x;
            vb[3 * i + 1] = vertex[i].y;
            vb[3 * i + 2] = vertex[i].z;
        }

        unsigned* ib = (unsigned*)rtcSetNewGeometryBuffer(
            rtc_geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
            3 * sizeof(unsigned), triangle_indices.size());
        for (int i = 0; i < triangle_indices.size(); i++) {
            ib[3 * i] = triangle_indices[i].x;
            ib[3 * i + 1] = triangle_indices[i].y;
            ib[3 * i + 2] = triangle_indices[i].z;
        }
    }

    /**
     * @brief Compute the normal at the hit position using barycentric
     * interpolation.
     * @param rayhit Information about the ray hit.
     * @param hit_pos The position of the hit.
     * @return The interpolated normal vector at the hit position.
     */
    vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos)
    {
        unsigned int prim_id = rayhit.hit.primID;
        glm::uvec3 face_nor = triangle_indices[prim_id];

        vec3 n1 = normal[face_nor.x];
        vec3 n2 = normal[face_nor.y];
        vec3 n3 = normal[face_nor.z];

        return glm::normalize(n2 * rayhit.hit.u + n3 * rayhit.hit.v + n1 * (1 - rayhit.hit.u - rayhit.hit.v));
    }

    std::string filename; /**< Filename of the OBJ file. */
    std::vector<vec3> normal; /**< Vertex normals. */
    std::vector<vec3> vertex; /**< Vertex positions. */
    std::vector<glm::uvec3>
        triangle_indices; /**< Indices of triangle vertices. */

protected:
    /**
     * @brief Link parameters with the Params struct.
     */
    void link_params()
    {
        params.add("filename", Params::Type::PATH, &filename);
        params.add("brdf", Params::Type::BRDF, &brdf);
        params.add("local_to_world", Params::Type::MAT4, &local_to_world);
    }
};

/**
 * @brief Class representing a sphere geometry.
 */
class Sphere : public Geometry {
public:
    /**
     * @brief Default constructor for Sphere.
     * Initializes the geometry type, position, and radius.
     */
    Sphere()
        : Geometry("Sphere")
        , pos(vec3(0.))
        , rad(1.)
    {
        link_params();

        this->brdf = std::shared_ptr<Brdf>(nullptr);
    };

    /**
     * @brief Initialize the sphere geometry.
     */
    void init() {};

    /**
     * @brief Initialize the Embree RTC geometry for the sphere.
     * @param device The Embree RTC device.
     */
    void init_rtc(RTCDevice device)
    {
        // Set embree
        rtc_geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_SPHERE_POINT);

        float* vb = (float*)rtcSetNewGeometryBuffer(
            rtc_geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4,
            4 * sizeof(float), 1);

        vb[0] = pos.x;
        vb[1] = pos.y;
        vb[2] = pos.z;
        vb[3] = rad;
    }

    /**
     * @brief Compute the normal at the hit position for the sphere.
     * @param rayhit Information about the ray hit.
     * @param hit_pos The position of the hit.
     * @return The normal vector at the hit position.
     */
    vec3 get_normal(RTCRayHit rayhit, const vec3& hit_pos)
    {
        return (hit_pos - pos) / rad;
    }

    vec3 pos; /**< Center position of the sphere. */
    float rad; /**< Radius of the sphere. */

protected:
    /**
     * @brief Link parameters with the Params struct.
     */
    void link_params()
    {
        params.add("pos", Params::Type::VEC3, &pos);
        params.add("rad", Params::Type::FLOAT, &rad);
        params.add("brdf", Params::Type::BRDF, &brdf);
        params.add("local_to_world", Params::Type::MAT4, &local_to_world);
    }
};

} // namespace LT_NAMESPACE
