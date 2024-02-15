#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <math.h>

#define LT_NAMESPACE lt

namespace LT_NAMESPACE{

	using vec3 = glm::vec3;
	
	using Spectrum = vec3;

	#define Float float
	
	const Float pi = 3.14159265359;

	template<typename T>
	inline std::vector<T> linspace(T start, T end, int size) {
		std::vector<T> arr;
		arr.resize(size);
		if (size == 0)
			return arr;

		T delta = (end - start) / (size - 1);
		for (int i = 0; i < size; i++)
			arr[i] = start + delta * i;

		return arr;
	}

	inline vec3 polar_to_card(Float theta, Float phi) {
		return vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
	}


	inline vec3 square_to_uniform_hemisphere(Float u1, Float u2) {
		Float z = u1;
		Float r = std::sqrt(std::max( 0. , 1. - z * z));;
		Float ph = 2. * pi * u2;
		return vec3(r * cos(ph), r * sin(ph), z);
	}

	inline Float square_to_uniform_hemisphere_pdf() {
		return 1. / (2. * pi);
	}

	inline vec3 square_to_cosine_hemisphere(Float u1, Float u2) {
		Float r = std::sqrt(u1);
		Float theta = 2. * pi * u2;
		Float dx = r * std::cos(theta);
		Float dy = r * std::sin(theta);
		Float z = std::sqrt(1. - dx* dx - dy*dy);
		return vec3(dx, dy, z);
	}

	inline Float square_to_cosine_hemisphere_pdf(vec3 w) {
		return w[2] / pi;
	}
	
}
