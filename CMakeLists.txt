cmake_minimum_required(VERSION 3.10)

project(panini)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

add_executable(panini-test panini.hpp panini-test.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /permissive-")
  target_compile_options(panini-test PRIVATE /W4 /WX)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions")
  target_compile_options(
    panini-test PRIVATE -Werror -pedantic -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo
  )
endif()

add_library(googletest STATIC googletest/googletest/src/gtest-all.cc)
target_compile_definitions(googletest PUBLIC GTEST_HAS_TR1_TUPLE=0 GTEST_LANG_CXX11=1)
target_include_directories(
  googletest
  PUBLIC googletest/googletest/include
  PRIVATE googletest/googletest
)
target_link_libraries(panini-test googletest)
