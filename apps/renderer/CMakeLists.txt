set(PROGRAM_NAME lil_tracer)

add_executable(${PROGRAM_NAME} main.cpp)

target_link_libraries(${PROGRAM_NAME} PRIVATE lil_tracer_lib)

find_package(OpenMP REQUIRED)
target_link_libraries(${PROGRAM_NAME} PRIVATE OpenMP::OpenMP_CXX)
