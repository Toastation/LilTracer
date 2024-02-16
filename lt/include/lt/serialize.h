#pragma once
#include <lt/lt_common.h>
#include <lt/params.h>

namespace LT_NAMESPACE {

	class Serializable {
	private:
		static int item_count;
	public:
		Serializable() {
			id = item_count;
			item_count++;
		};

		int id;
		Params params;
	protected :
		virtual void link_params() = 0;
	};

}