set(PROGRAM_NAME renderer)

add_executable(${PROGRAM_NAME} main.cpp)

target_link_libraries( ${PROGRAM_NAME} PUBLIC lil_tracer)

target_include_directories(${PROGRAM_NAME}  PUBLIC ../../3rd_party/)

find_package(OpenMP REQUIRED)
target_link_libraries(${PROGRAM_NAME} PRIVATE OpenMP::OpenMP_CXX)

message(${CMAKE_SOURCE_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})


add_custom_command(
        TARGET ${PROGRAM_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data
                ${CMAKE_CURRENT_BINARY_DIR}/data)
