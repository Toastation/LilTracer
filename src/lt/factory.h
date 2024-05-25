/**
 * @file
 * @brief Definition of the Factory class template.
 */

#pragma once
#include <lt/lt_common.h>

#include <functional>
#include <map>

namespace LT_NAMESPACE {

/**
 * @brief Template class for creating objects using a factory pattern.
 * @tparam T The base type of objects to be created.
 */
template <class T>
class Factory {
public:
    using Creator = std::function<std::shared_ptr<T>()>; /**< Alias for creator
                                                            function type. */
    using CreatorRegistry = std::map<const std::string,
        Creator>; /**< Alias for registry of creators. */

    /**
     * @brief Accessor for the creator registry.
     * @return A reference to the creator registry.
     */
    static CreatorRegistry& registry();

    /**
     * @brief Get the names of registered objects in the factory.
     * @return A vector containing the names of registered objects.
     */
    static std::vector<std::string> names()
    {
        std::vector<std::string> names;
        for (auto it = registry().begin(); it != registry().end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

    /**
     * @brief Create an object of type T with the given name.
     * @tparam Ts Parameter pack for constructor arguments.
     * @param name The name of the object to create.
     * @param ts Constructor arguments.
     * @return A shared pointer to the created object.
     */
    template <class... Ts>
    static std::shared_ptr<T> create(const std::string& name, Ts&&... ts)
    {
        typename CreatorRegistry::iterator it = registry().find(name);

        if (it != registry().end()) {
            Creator creator = it->second;
            return creator(std::forward<Ts>(ts)...);
        }

        Log(logError) << "factory registry[" << name << "] undefined";
        return std::shared_ptr<T>(nullptr);
    }
};

} // namespace LT_NAMESPACE