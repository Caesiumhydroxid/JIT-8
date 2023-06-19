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

#SET(GCC_COVERAGE_COMPILE_FLAGS "-fsanitize=address")
#set(CMAKE_BUILD_TYPE Debug)
#add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable(CMakeSFMLProject ${SOURCES})

target_link_libraries(CMakeSFMLProject sfml-graphics asmjit)
target_compile_features(CMakeSFMLProject PRIVATE cxx_std_17)

install(TARGETS CMakeSFMLProject)