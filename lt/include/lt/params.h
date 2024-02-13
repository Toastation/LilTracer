#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {

	struct Params {
		enum class Type
		{
			FLOAT,
			VEC3
		};

		int count = 0;
		std::vector<void*> ptrs;
		std::vector<Type> types;
		std::vector<std::string> names;

		void add(const std::string& name, Type type, void* ptr) {
			count++;
			ptrs.push_back(ptr);
			types.push_back(type);
			names.push_back(name);
		}
	};

}