#pragma once
#include <lt/lt_common.h>
#include <lt/params.h>

namespace LT_NAMESPACE {

	class Serializable {
	private:
		static int item_count;
	public:

		Serializable(const std::string& type) : type(type) {
			id = item_count;
			item_count++;
		};

		virtual void init() {};

		int id;
		Params params;
		std::string type;
	protected :
		virtual void link_params() = 0;
	};

}