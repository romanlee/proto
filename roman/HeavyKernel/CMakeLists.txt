blt_add_executable(NAME HeavyKernel SOURCES HeavyKernel.cpp
    DEPENDS_ON Headers_Base ${LIB_DEP} gtest
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
  blt_add_test(NAME HeavyKernel COMMAND HeavyKernel)
