

cmake_minimum_required(VERSION 3.9)

project(LilTracer 
  VERSION 1.0.0
  LANGUAGES CXX C
)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(clang_format)

# ----------------------------------------------------------------------------
# LilTracer library
# ----------------------------------------------------------------------------

add_subdirectory(lt)
add_clang_format_target(format-liltracer lt)


# ----------------------------------------------------------------------------
# Applications
# ----------------------------------------------------------------------------

add_subdirectory(apps/brdf_viewer)

add_subdirectory(apps/renderer)


# ----------------------------------------------------------------------------
# Documentation
# ----------------------------------------------------------------------------

add_subdirectory(doc)