#pragma once
#include <lt/lt_common.h>

#include <functional>
#include <map>

namespace LT_NAMESPACE {

	template <class T>
	class Factory {
	public:

		using  Creator = std::function<std::shared_ptr<T>()>;
		using  CreatorRegistry = std::map<const std::string, Creator>;
		static CreatorRegistry& registry();

		static std::vector<std::string> names() {
			std::vector<std::string> names;
			for (auto it = registry().begin(); it != registry().end(); ++it) {
				names.push_back(it->first);
			}
			return names;
		}

		template <class ...Ts>
		static std::shared_ptr<T> create(const std::string& name, Ts&&...ts)
		{

			typename CreatorRegistry::iterator it = registry().find(name);

			if (it != registry().end()) {
				Creator creator = it->second;
				return creator(std::forward<Ts>(ts)...);
			}

			std::cerr << "factory registry[" << name << "] undefined" << std::endl;
			return std::shared_ptr<T>(nullptr);
		}
	};



}