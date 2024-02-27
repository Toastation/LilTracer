/**
 * @file
 * @brief Definition of the Ray class.
 */

#pragma once

#include <lt/lt_common.h>

namespace LT_NAMESPACE {

    /**
     * @brief Ray.
     */
	class Ray
	{
	public:
		/**
         * @brief Default constructor for Ray.
         * Initializes origin and direction to zero vectors.
         */
		Ray() : o(vec3(0.)), d(vec3(0.)) {}

		/**
         * @brief Constructor for Ray.
         * @param o The origin vector.
         * @param d The direction vector.
         * Initializes origin and direction with given parameters.
         */
		Ray(vec3 o, vec3 d) :
			o(o),
			d(d){}

		vec3 o; /**< Origin vector. */
		vec3 d; /**< Direction vector. */


	};

}