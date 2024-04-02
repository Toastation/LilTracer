#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>

#include <lt/brdf/brdf.h>
#include <lt/brdf/lambert.h>
#include <lt/brdf/emissive.h>
#include <lt/brdf/test.h>
#include <lt/brdf/shape_invariant.h>
#include <lt/brdf/mix.h>

namespace LT_NAMESPACE {
	



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

        static BrdfValidation validate(const Brdf& brdf) {
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


} // end  namespace LT_NAMESPACE