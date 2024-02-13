#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <math.h>

#define LT_NAMESPACE lt

namespace LT_NAMESPACE{

	using vec3 = glm::vec3;
	

	template<typename T>
	inline std::vector<T> linspace(T start, T end, int size) {
		std::vector<T> arr;
		arr.resize(size);
		if (size == 0)
			return arr;

		T delta = (end - start) / size;
		for (int i = 0; i < size; i++)
			arr[i] = start + delta * i;

		return arr;
	}

	inline vec3 polar_to_card(float theta, float phi) {
		return vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
	}

	const float pi = 3.14159265359;
}
