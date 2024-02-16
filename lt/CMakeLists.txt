
include_directories(include)

file(GLOB cpp_h_files
     "include/lt/*.h"
     "src/*.cpp"
)



add_library(lil_tracer STATIC ${cpp_h_files} )
target_include_directories(lil_tracer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


find_package(glm CONFIG REQUIRED)
target_link_libraries(lil_tracer PRIVATE glm::glm)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(lil_tracer PRIVATE nlohmann_json nlohmann_json::nlohmann_json)



