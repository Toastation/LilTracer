

cmake_minimum_required(VERSION 3.9)

project(LilTracer 
  VERSION 1.0.0
  LANGUAGES CXX C
)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# LilTracer library
# ----------------------------------------------------------------------------

add_subdirectory(lt)


# ----------------------------------------------------------------------------
# Applications
# ----------------------------------------------------------------------------

add_subdirectory(apps/brdf_viewer)