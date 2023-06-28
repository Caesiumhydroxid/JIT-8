cmake_minimum_required(VERSION 3.21)
project(CMakeSFMLProject LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.5.1)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(ASMJIT
    GIT_REPOSITORY https://github.com/asmjit/asmjit)
FetchContent_MakeAvailable(ASMJIT)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wextra -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

#SET(GCC_COVERAGE_COMPILE_FLAGS "-fsanitize=address")
#set(CMAKE_BUILD_TYPE Debug)
#add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable(jit8 ${SOURCES})

target_link_libraries(jit8 sfml-graphics asmjit)
target_compile_features(jit8 PRIVATE cxx_std_17)

install(TARGETS jit8)