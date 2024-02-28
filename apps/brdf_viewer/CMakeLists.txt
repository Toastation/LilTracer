set(PROGRAM_NAME brdf_viewer)

add_executable(${PROGRAM_NAME} main.cpp)

target_link_libraries( ${PROGRAM_NAME} PUBLIC lil_tracer)

target_include_directories(${PROGRAM_NAME}  PUBLIC ../../3rd_party/)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(brdf_viewer PRIVATE glfw)

find_package(GLEW CONFIG REQUIRED)
target_link_libraries(brdf_viewer PRIVATE GLEW::GLEW)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(brdf_viewer PRIVATE imgui::imgui)

find_package(implot CONFIG REQUIRED)
target_link_libraries(brdf_viewer PRIVATE implot::implot)

find_package(OpenMP REQUIRED)
target_link_libraries(brdf_viewer PRIVATE OpenMP::OpenMP_CXX)