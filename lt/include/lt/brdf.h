/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>
#include <lt/sampler.h>
#include <lt/serialize.h>

namespace LT_NAMESPACE {
#define PARAMETER(type, name, default_values) type name = type(default_values)

/**
 * @brief Base class for Bidirectional Reflectance Distribution Functions
 * (BRDFs).
 */
class Brdf : public Serializable {
public:

    struct Sample {
        vec3 wo;
        Spectrum value; // brdf / pdf
    };

    enum class flags
    {
        rough = 1 << 0,
        specular = 1 << 1
    };

    /**
     * @brief Constructor.
     * @param type The type of the BRDF.
     */
    Brdf(const std::string& type)
        : Serializable(type) 
    {
        emissive = false;
    };

    /**
     * @brief Evaluates the BRDF.
     * @param wi Incident direction.
     * @param wo Outgoing direction.
     * @return The evaluated spectrum.
     */
    virtual Spectrum eval(vec3 wi, vec3 wo);

    /**
     * @brief Samples the BRDF.
     * @param wi Incident direction.
     * @param sampler The sampler object used for sampling.
     * @return The sampled direction.
     */
    virtual Sample sample(const vec3& wi, Sampler& sampler);

    /**
     * @brief Computes the density of a sample of wo.
     * @param wo Direction.
     * @return The density value.
     */
    virtual float pdf(const vec3& wi, const vec3& wo);

    bool emissive;
    virtual Spectrum emission();
};

class Emissive : public Brdf {
public:
    PARAMETER(Spectrum, intensity, 1.); /**< Intensity of emission. */

    Emissive()
        : Brdf("Emissive")
    {
        emissive = true;
        link_params();
    }

    Spectrum emission();

protected:
    void link_params()
    {
        params.add("intensity", Params::Type::VEC3, &intensity);
    }
};

/**
 * @brief Diffuse BRDF class.
 */
class Diffuse : public Brdf {
public:
    PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

    Diffuse()
        : Brdf("Diffuse")
    {
        link_params();
    }

    Spectrum eval(vec3 wi, vec3 wo);

protected:
    void link_params() { params.add("albedo", Params::Type::VEC3, &albedo); }
};

/**
 * @brief Test BRDF class for demonstration purposes.
 */
class TestBrdf : public Brdf {
public:
    PARAMETER(float, v1, 0.5); /**< Test parameter 1. (float) */
    PARAMETER(Spectrum, v2, 0.5); /**< Test parameter 2. (Spectrum) */
    PARAMETER(std::vector<float>, v3, 0); /**< Test parameter 3. (Array<float>) */

    TestBrdf()
        : Brdf("TestBrdf")
    {
        v3.push_back(2.);
        v3.push_back(1.);
        v3.push_back(3.);
        v3.push_back(4.);
        v3.push_back(9.);
        link_params();
    }

    Spectrum eval(vec3 wi, vec3 wo);

protected:
    void link_params()
    {
        params.add("float", Params::Type::FLOAT, &v1);
        params.add("vec3", Params::Type::VEC3, &v2);
        params.add("array", Params::Type::SH, &v3);
    }
};

template <class MICROSURFACE>
class ShapeInvariantMicrosurface : public Brdf {
public:
    vec3 scale;

    ShapeInvariantMicrosurface(const std::string& type, const Float& scale_x,
        const Float& scale_y)
        : Brdf(type)
    {
        scale = vec3(scale_x, scale_y, 1.);
        sample_visible_distribution = false;
    }


    Float scale_wi(const vec3& wi) const;
    vec3 to_unit_space(const vec3& wi);
    vec3 to_transformed_space(const vec3& wi);
    
    Float G1(const vec3& wh, const vec3& wi);

    Float D(const vec3& wh);
    Float pdf_wh(const vec3& wh);
    
    Float D(const vec3& wh, const vec3& wi);
    Float pdf_wh(const vec3& wh, const vec3& wi);
    
    Float pdf(const vec3& wi, const vec3& wo);

    vec3 sample_D(Sampler& sampler);
    vec3 sample_D(const vec3& wi, Sampler& sampler);
    //Sample sample(const vec3& wi, Sampler& sampler);

    //virtual Spectrum eval(vec3 wi, vec3 wo) = 0;

    MICROSURFACE ms;
    bool sample_visible_distribution;
};

class SphereMicrosurface {
public:
    Float D(const vec3& wh_u);
    Float D(const vec3& wh_u, const vec3& wi_u);
     
    Float pdf(const vec3& wh_u);
    Float pdf(const vec3& wh_u, const vec3& wi_u);
  
    vec3 sample_D(Sampler& sampler);
    // Sampling method from Sampling Visible GGX Normals with Spherical Caps, Jonathan Dupuy, Anis Benyoub
    vec3 sample_D(const vec3& wi_u, Sampler& sampler);
    
    Float lambda(const vec3& wi_u);
    Float G1(const vec3& wh_u, const vec3& wi_u);
};

template <class MICROSURFACE>
class RoughShapeInvariantMicrosurface : public ShapeInvariantMicrosurface<SphereMicrosurface> {
public:
    RoughShapeInvariantMicrosurface(const std::string& type, const Float& scale_x,
        const Float& scale_y)
        : ShapeInvariantMicrosurface<SphereMicrosurface>(type, scale_x, scale_y)
    {
        eta = Spectrum(1.);
        kappa = Spectrum(10000.);
    }

    Spectrum eval(vec3 wi, vec3 wo);
    Sample sample(const vec3& wi, Sampler& sampler);

    Spectrum eta;
    Spectrum kappa;
};


class RoughGGX : public RoughShapeInvariantMicrosurface<SphereMicrosurface> {
public:
    RoughGGX()
        : RoughShapeInvariantMicrosurface<SphereMicrosurface>("RoughGGX", 0.1,
            0.1)
    {
        link_params();
    }

    RoughGGX(const Float& scale_x, const Float& scale_y)
        : RoughShapeInvariantMicrosurface<SphereMicrosurface>("RoughGGX",
            scale_x, scale_y)
    {
        link_params();
    }
    
protected:
    void link_params()
    {
        params.add("rough_x", Params::Type::FLOAT, &scale[0]);
        params.add("rough_y", Params::Type::FLOAT, &scale[1]);
        params.add("eta", Params::Type::IOR, &eta);
        params.add("kappa", Params::Type::IOR, &kappa);
        params.add("sample_visible_distribution", Params::Type::BOOL, &sample_visible_distribution);
    }
};


class BeckmannMicrosurface {
public:
    Float D(const vec3& wh_u);
    Float D(const vec3& wh_u, const vec3& wi_u);

    Float pdf(const vec3& wh_u);
    Float pdf(const vec3& wh_u, const vec3& wi_u);

    vec3 sample_D(Sampler& sampler);
    vec3 sample_D(const vec3& wi_u, Sampler& sampler);

    Float lambda(const vec3& wi_u);
    Float G1(const vec3& wh_u, const vec3& wi_u);
};


class RoughBeckmann : public RoughShapeInvariantMicrosurface<BeckmannMicrosurface> {
public:
    RoughBeckmann()
        : RoughShapeInvariantMicrosurface<BeckmannMicrosurface>("RoughBeckmann", 0.1, 0.1)
    {
        link_params();
    }

    RoughBeckmann(const Float& scale_x, const Float& scale_y)
        : RoughShapeInvariantMicrosurface<BeckmannMicrosurface>("RoughBeckmann", scale_x, scale_y)
    {
        link_params();
    }

protected:
    void link_params()
    {
        params.add("rough_x", Params::Type::FLOAT, &scale[0]);
        params.add("rough_y", Params::Type::FLOAT, &scale[1]);
        params.add("eta", Params::Type::IOR, &eta);
        params.add("kappa", Params::Type::IOR, &kappa);
    }
};




struct BrdfValidation {
    std::vector<Float> directionnal_albedo;
    bool energy_conservative; // all  directionnal_albedo <  1
    bool correct_sampling;    // sample = pdf
    bool reciprocity;
    bool found_nan;
    bool negative_value;

    BrdfValidation() : 
        energy_conservative(false),
        correct_sampling(false),
        reciprocity(true),
        found_nan(false),
        negative_value(false)
    {}
    
    static BrdfValidation validate(const Brdf& brdf){
        std::cout << "validate " << brdf.type << std::endl;
        return BrdfValidation();
    }
};



} // namespace LT_NAMESPACE