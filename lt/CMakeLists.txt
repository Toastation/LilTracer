set(PROGRAM_NAME lil_tracer)

include_directories(include)

file(GLOB cpp_h_files
     "include/lt/*.h"
     "src/*.cpp"
)


add_library(${PROGRAM_NAME}  STATIC ${cpp_h_files} )
target_include_directories(${PROGRAM_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ../3rd_party/fast_obj)



add_library(fast_obj_lib STATIC ../3rd_party/fast_obj/fast_obj.c ../3rd_party/fast_obj/fast_obj.h)
target_link_libraries(${PROGRAM_NAME}  PRIVATE fast_obj_lib)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROGRAM_NAME}  PRIVATE glm::glm)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROGRAM_NAME}  PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(embree 3 CONFIG REQUIRED)
target_link_libraries(${PROGRAM_NAME} PRIVATE embree)
