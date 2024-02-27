#
# DOCUMENTATION
#
 
# Auto documentation
## Doxygen
find_package(Doxygen REQUIRED)

if(DOXYGEN)
  add_custom_target(
    doc
    COMMAND ${DOXYGEN} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    COMMAND cd docs &amp ;&amp ; make html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()