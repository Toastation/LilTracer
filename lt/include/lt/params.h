/**
 * @file
 * @brief Definition of the Params class.
 */

#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {

/**
 * @brief Struct for storing parameters.
 */
struct Params {
  /**
   * @brief Enum defining the types of parameters.
   */
  enum class Type {
    FLOAT,  /**< Float type parameter. */
    VEC3,   /**< Vector3 type parameter. */
    SH,     /**< Spherical Harmonics type parameter. */
    PATH,   /**< Path type parameter. */
    BRDF,   /**< BRDF type parameter. */
    TEXTURE /**< Texture type parameter. */
  };

  int count = 0;                  /**< Count of parameters. */
  std::vector<void *> ptrs;       /**< Pointers to parameter values. */
  std::vector<Type> types;        /**< Types of parameters. */
  std::vector<std::string> names; /**< Names of parameters. */

  /**
   * @brief Add a parameter to the Params struct.
   * @param name The name of the parameter.
   * @param type The type of the parameter.
   * @param ptr A pointer to the parameter value.
   */
  void add(const std::string &name, Type type, void *ptr) {
    count++;
    ptrs.push_back(ptr);
    types.push_back(type);
    names.push_back(name);
  }
};

}  // namespace LT_NAMESPACE