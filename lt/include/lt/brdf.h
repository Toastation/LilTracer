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

enum class BrdfFlags : uint16_t
{
    rough = 1 << 0,
    specular = 1 << 1,
    diffuse = 1 << 2,
    reflection = 1 << 3,
    transmission = 1 << 4,
    emissive = 1 << 5
};

inline BrdfFlags operator|(const BrdfFlags& lhs, const BrdfFlags& rhs)
{
    return (BrdfFlags)(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

inline BrdfFlags operator&(const BrdfFlags& lhs, const BrdfFlags& rhs)
{
    return (BrdfFlags)(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

inline bool is_emissive(const BrdfFlags& flags) {
    return static_cast<uint16_t>(flags) & static_cast<uint16_t>(BrdfFlags::emissive);
}


/**
 * @brief Base class for Bidirectional Reflectance Distribution Functions
 * (BRDFs).
 */
class Brdf : public Serializable {
public:



    struct Sample {
        vec3 wo;
        Spectrum value; // brdf / pdf
        BrdfFlags flags;
    };

    /**
     * @brief Constructor.
     * @param type The type of the BRDF.
     */
    Brdf(const std::string& type)
        : Serializable(type) 
    {};

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
    
    BrdfFlags flags;
    virtual Spectrum emission();
};

class Emissive : public Brdf {
public:
    PARAMETER(Spectrum, intensity, 1.); /**< Intensity of emission. */

    Emissive()
        : Brdf("Emissive")
    {
        flags = BrdfFlags::emissive;
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
        flags = BrdfFlags::diffuse | BrdfFlags::reflection;
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
    Float G2(const vec3& wh, const vec3& wi, const vec3& wo);

    Float D(const vec3& wh);
    Float pdf_wh(const vec3& wh);
    
    Float D(const vec3& wh, const vec3& wi);
    Float pdf_wh(const vec3& wh, const vec3& wi);
    

    vec3 sample_D(Sampler& sampler);
    vec3 sample_D(const vec3& wi, Sampler& sampler);
    
    // Implemented in child class
    // Sample sample(const vec3& wi, Sampler& sampler);
    // Spectrum eval(vec3 wi, vec3 wo) = 0;
    // Float pdf(const vec3& wi, const vec3& wo);

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
    Float G2(const vec3& wh_u, const vec3& wi_u, const vec3& wo_u);
};

template <class MICROSURFACE>
class RoughShapeInvariantMicrosurface : public ShapeInvariantMicrosurface<MICROSURFACE> {
public:
    RoughShapeInvariantMicrosurface(const std::string& type, const Float& scale_x, const Float& scale_y)
        : ShapeInvariantMicrosurface<MICROSURFACE>(type, scale_x, scale_y)
    {
        Brdf::flags = BrdfFlags::rough | BrdfFlags::reflection;
        eta = Spectrum(1.);
        kappa = Spectrum(10000.);
    }

    Spectrum eval(vec3 wi, vec3 wo);
    Brdf::Sample sample(const vec3& wi, Sampler& sampler);
    Float pdf(const vec3& wi, const vec3& wo);

    Spectrum eta;
    Spectrum kappa;
};


class RoughGGX : public RoughShapeInvariantMicrosurface<SphereMicrosurface> {
public:
    RoughGGX()
        : RoughShapeInvariantMicrosurface<SphereMicrosurface>("RoughGGX", 0.1, 0.1)
    {
        link_params();
    }

    RoughGGX(const Float& scale_x, const Float& scale_y)
        : RoughShapeInvariantMicrosurface<SphereMicrosurface>("RoughGGX", scale_x, scale_y)
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
    Float G2(const vec3& wh_u, const vec3& wi_u, const vec3& wo_u);
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


class Mix : public Brdf {
public:
    PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

    Mix() : Brdf("Mix")
    {
        flags = BrdfFlags::diffuse | BrdfFlags::reflection;
        link_params();
    }

    void init() {
        flags = brdf1->flags | brdf2->flags;
    }

    Spectrum eval(vec3 wi, vec3 wo) {
        return weight * brdf1->eval(wi, wo) + (1.f - weight) * brdf2->eval(wi, wo);
    }

    std::shared_ptr<Brdf> brdf1;
    std::shared_ptr<Brdf> brdf2;
    Float weight;

protected:
    void link_params() 
    { 
        params.add("brdf1", Params::Type::BRDF, &brdf1);
        params.add("brdf2", Params::Type::BRDF, &brdf2);
        params.add("weight", Params::Type::FLOAT, &weight);
    }
};



struct BrdfValidation {
    static int number_of_sample;
    static int number_of_theta;

    std::vector<Float> directional_albedo;
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

        // Compute directional albedo
        
            // Check if there is negative values

        
        // Check if all directional albedo are < 1 (energy conservative)




        // Test specific directions to found nan
        // wi = wo

        

        // Test reciprocity


        // Test sampling
        

        //float dtheta = 0.5 * lt::pi / app_data.s_brdf_sampling->h;
        //float dphi = 2. * lt::pi / app_data.s_brdf_sampling->w;
        //float sum_1 = 0.;
        //float sum_2 = 0.;
        //float sum_3 = 0.;
        //for (int x = 0; x < app_data.s_brdf_sampling_pdf->w; x++) {
        //    for (int y = 0; y < app_data.s_brdf_sampling_pdf->h; y++) {
        //        lt::vec3 wo = lt::polar_to_card(th[y], ph[x] );
        //        float theta = 0.5 * lt::pi * (float(y) + 0.5) / app_data.s_brdf_sampling_pdf->h;
        //        sum_1 += app_data.s_brdf_sampling->get(x, y).x * std::sin(theta) * dtheta * dphi;
        //        sum_2 += app_data.s_brdf_sampling_pdf->get(x, y).x * std::sin(theta) * dtheta * dphi;
        //        sum_3 += lt::square_to_cosine_hemisphere_pdf(wo) * std::sin(theta) * dtheta * dphi;
        //    }
        //}

        //std::cout << sum_1 << std::endl;
        //std::cout << sum_2 << std::endl;
        //std::cout << sum_3 << std::endl;

        return BrdfValidation();
    }



};



} // namespace LT_NAMESPACE