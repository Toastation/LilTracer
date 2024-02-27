/**
 * @file
 * @brief Definition of the Sampler class.
 */

#pragma once 
#include <lt/lt_common.h>
#include <random>

namespace LT_NAMESPACE {

    /**
     * @brief Class for generating random samples.
     */
	class Sampler
	{
	public:

        /**
         * @brief Default constructor.
         * Initializes the random number generator and uniform distribution.
         */
		Sampler() {
			_rng = std::mt19937(_dev());
			_urd = std::uniform_real_distribution<Float>(0, 1);
		};

		/**
         * @brief Generate a random float.
         * @return A random float.
         */
		Float next_float() {
			return _urd(_rng);
		}

		/**
         * @brief Change the seed of the random number generator.
         * @param s The seed value.
         */
		void seed(uint32_t s) {
			_rng.seed(s);
		}

	private:
		std::random_device _dev; 						/**< Random device for seeding. */
		std::mt19937 _rng; 								/**< Mersenne Twister random number generator. */
		std::uniform_real_distribution<Float> _urd; 	/**< Uniform real distribution. */
	};

}