# cmake/FindReflectCpp.cmake
#
# Usage:
#   find_package(ReflectCpp REQUIRED)
#   target_link_libraries(your_target PRIVATE ${ReflectCpp_TARGET})
#
# Exposes:
#   ReflectCpp_FOUND        - TRUE/FALSE
#   ReflectCpp_ROOT_DIR     - path to third_party/reflect-cpp
#   ReflectCpp_INCLUDE_DIR  - public include dir
#   ReflectCpp_VERSION      - reflect-cpp version string (best effort)
#   ReflectCpp_TARGET       - target name to link against (reflectcpp::reflectcpp)
#
# Behavior:
#   - Assumes reflect-cpp sources are vendored at:
#         ${CMAKE_SOURCE_DIR}/third_party/reflect-cpp
#   - Disables tests / benchmarks / install from reflect-cpp
#   - Adds reflect-cpp via add_subdirectory(...) exactly once
#
# Requirements:
#   - This module is found via CMAKE_MODULE_PATH before calling find_package(ReflectCpp)
#   - project() must already be called in the root CMakeLists.txt

include_guard()

# Expected vendor location
set(_REFLECTCPP_CANDIDATE_DIR "${CMAKE_SOURCE_DIR}/third_party/reflect-cpp")

# If reflect-cpp is not there -> report not found and bail
if(NOT EXISTS "${_REFLECTCPP_CANDIDATE_DIR}/CMakeLists.txt")
    set(ReflectCpp_FOUND FALSE)
    set(ReflectCpp_ROOT_DIR "")
    set(ReflectCpp_INCLUDE_DIR "")
    set(ReflectCpp_VERSION "")
    set(ReflectCpp_TARGET "")
    message(STATUS "[FindReflectCpp] reflect-cpp not found at: ${_REFLECTCPP_CANDIDATE_DIR}")
    return()
endif()

# Disable stuff we don't want to build from reflect-cpp when vendored
set(REFLECTCPP_BUILD_TESTS OFF CACHE BOOL
        "Disable reflect-cpp tests for vendored build"
        FORCE)

set(REFLECTCPP_BUILD_BENCHMARKS OFF CACHE BOOL
        "Disable reflect-cpp benchmarks for vendored build"
        FORCE)

set(REFLECTCPP_INSTALL OFF CACHE BOOL
        "Disable reflect-cpp install for vendored build"
        FORCE)

# reflect-cpp by default pulls its own bundled deps (yyjson etc).
# We don't override REFLECTCPP_JSON / REFLECTCPP_USE_BUNDLED_DEPENDENCIES.

# Add reflect-cpp subdirectory only once
if(NOT TARGET reflectcpp::reflectcpp)
    add_subdirectory(
            "${_REFLECTCPP_CANDIDATE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/third_party/reflect-cpp"
            EXCLUDE_FROM_ALL
    )
endif()

# Public info
set(ReflectCpp_ROOT_DIR    "${_REFLECTCPP_CANDIDATE_DIR}")
set(ReflectCpp_INCLUDE_DIR "${_REFLECTCPP_CANDIDATE_DIR}/include")
set(ReflectCpp_TARGET      reflectcpp::reflectcpp)

# Try to infer version.
# reflect-cpp typically does: project(reflectcpp VERSION X.Y.Z ...)
if(DEFINED reflectcpp_VERSION)
    set(ReflectCpp_VERSION "${reflectcpp_VERSION}")
else()
    get_target_property(_rfl_soversion reflectcpp::reflectcpp SOVERSION)
    if(_rfl_soversion)
        set(ReflectCpp_VERSION "${_rfl_soversion}")
    else()
        set(ReflectCpp_VERSION "")
    endif()
endif()

set(ReflectCpp_FOUND TRUE)

mark_as_advanced(
        ReflectCpp_ROOT_DIR
        ReflectCpp_INCLUDE_DIR
        ReflectCpp_VERSION
        ReflectCpp_TARGET
)

unset(_REFLECTCPP_CANDIDATE_DIR)
