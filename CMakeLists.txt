cmake_minimum_required(VERSION 3.16)
project(mini_unique_ptr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unique_ptr)
set(INC_DIR  ${PROJ_DIR}/include)
set(TEST_DIR ${PROJ_DIR}/tests)

# ---- Header-only library ----
add_library(mini_unique_ptr INTERFACE)
target_sources(mini_unique_ptr INTERFACE ${INC_DIR}/mini_unique_ptr.hpp)
target_include_directories(mini_unique_ptr INTERFACE ${INC_DIR})

# ---- Test executable ----
add_executable(day1_smoke ${TEST_DIR}/smoke.cpp)
target_link_libraries(day1_smoke PRIVATE mini_unique_ptr)

# Warnings
if (MSVC)
  target_compile_options(day1_smoke PRIVATE /W4 /permissive- /EHsc)
else()
  target_compile_options(day1_smoke PRIVATE -Wall -Wextra -Wconversion -Werror -pedantic -O2 -fsized-deallocation)
endif()

# Remove-Item -Recurse -Force .\build
# cmake -S . -B build          # reconfigure; this refreshes the .vcxproj without the .cpp
# cmake --build build --config Debug -j