cmake_minimum_required(VERSION 3.14)

project(jouled VERSION 0.1.0 LANGUAGES C)

# include(CTest)
# enable_testing()
# add_executable(jouled_test jouled.test.c)
# target_link_libraries(jouled_test jouled)
# add_test(NAME jouled_test COMMAND jouled_test)

# Add libpfm library to the linker
set(LIBPFM_LIBRARY "libpfm.a")

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -O1")

# Add jouled library
add_library(jouled jouled.c perf_util.c)
target_link_libraries(jouled ${LIBPFM_LIBRARY})
set_target_properties(jouled PROPERTIES PUBLIC_HEADER jouled.h)
set_target_properties(jouled PROPERTIES POSITION_INDEPENDENT_CODE ON)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

add_executable(sonde sonde.c)
target_link_libraries(sonde jouled ${LIBPFM_LIBRARY})
