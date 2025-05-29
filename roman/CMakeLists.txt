blt_add_executable(NAME LightKernel SOURCES LightKernel.cpp
    DEPENDS_ON Headers_Base ${LIB_DEP} gtest
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
  blt_add_test(NAME LightKernel COMMAND LightKernel)