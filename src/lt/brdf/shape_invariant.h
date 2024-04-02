/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include "brdf.h"

namespace LT_NAMESPACE {

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
    bool optimize;
};

template <class MICROSURFACE>
class RoughShapeInvariantMicrosurface : public ShapeInvariantMicrosurface<MICROSURFACE> {
public:
    RoughShapeInvariantMicrosurface(const std::string& type, const Float& scale_x, const Float& scale_y)
        : ShapeInvariantMicrosurface<MICROSURFACE>(type, scale_x, scale_y)
    {
        Brdf::flags = Brdf::Flags::rough | Brdf::Flags::reflection;
        eta = Spectrum(1.);
        kappa = Spectrum(10000.);
    }

    Spectrum eval(vec3 wi, vec3 wo, Sampler& sampler);
    Brdf::Sample sample(const vec3& wi, Sampler& sampler);
    Float pdf(const vec3& wi, const vec3& wo);

    // Return eval / pdf
    Spectrum eval_optim(vec3 wi, vec3 wo, Sampler& sampler);

    Spectrum eta;
    Spectrum kappa;
};


template <class MICROSURFACE>
class DiffuseShapeInvariantMicrosurface : public ShapeInvariantMicrosurface<MICROSURFACE> {
public:
    DiffuseShapeInvariantMicrosurface(const std::string& type, const Float& scale_x, const Float& scale_y)
        : ShapeInvariantMicrosurface<MICROSURFACE>(type, scale_x, scale_y)
    {
        Brdf::flags = Brdf::Flags::diffuse | Brdf::Flags::reflection;
        albedo = Spectrum(0.5);
    }

    Spectrum eval(vec3 wi, vec3 wo, Sampler& sampler);
    Brdf::Sample sample(const vec3& wi, Sampler& sampler);
    Float pdf(const vec3& wi, const vec3& wo);

    Spectrum albedo;
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



class DiffuseGGX : public DiffuseShapeInvariantMicrosurface<SphereMicrosurface> {
public:
    DiffuseGGX()
        : DiffuseShapeInvariantMicrosurface<SphereMicrosurface>("DiffuseGGX", 0.1, 0.1)
    {
        link_params();
    }

    DiffuseGGX(const Float& scale_x, const Float& scale_y)
        : DiffuseShapeInvariantMicrosurface<SphereMicrosurface>("DiffuseGGX", scale_x, scale_y)
    {
        link_params();
    }

protected:
    void link_params()
    {
        params.add("rough_x", Params::Type::FLOAT, &scale[0]);
        params.add("rough_y", Params::Type::FLOAT, &scale[1]);
        params.add("albedo", Params::Type::VEC3, &albedo);
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



} // namespace LT_NAMESPACE