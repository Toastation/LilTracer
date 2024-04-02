/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include "mix.h"

namespace LT_NAMESPACE {

class Mix : public Brdf {
public:
    PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

    Mix() : Brdf("Mix")
    {
        link_params();
    }

    void init() {
        flags = brdf1->flags | brdf2->flags;
    }

    Spectrum eval(vec3 wi, vec3 wo, Sampler& sampler) {
        return weight * brdf1->eval(wi, wo, sampler) + (1.f - weight) * brdf2->eval(wi, wo, sampler);
    }


    Sample sample(const vec3& wi, Sampler& sampler) 
    {
        Sample bs;
        if (sampler.next_float() < weight) {
            bs = brdf1->sample(wi, sampler);
            bs.value /= weight;
        }
        else {
            bs = brdf2->sample(wi, sampler);
            bs.value /= 1. - weight;
        }
        return bs;
    }

    Float pdf(const vec3& wi, const vec3& wo) {
        return weight * brdf1->pdf(wi, wo) + (1.f - weight) * brdf2->pdf(wi, wo);
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


} // namespace LT_NAMESPACE