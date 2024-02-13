
include_directories(include)

file(GLOB cpp_h_files
     "include/lt/*.h"
     "src/*.cpp"
)

#add_library( stb_image STATIC ../3rd_party/stb/stb_image.h ../3rd_party/stb/stb_image_write.h )
#target_include_directories( stb_image PUBLIC ../3rd_party/stb)
#
#target_link_libraries(virtual_gonio stb_image)


add_library(lil_tracer STATIC ${cpp_h_files} )
target_include_directories(lil_tracer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


find_package(glm CONFIG REQUIRED)
target_link_libraries(lil_tracer PRIVATE glm::glm)



