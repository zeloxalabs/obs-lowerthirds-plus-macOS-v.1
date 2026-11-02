cmake_minimum_required(VERSION 3.16...3.26)

project(LowerThirdsPlus VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OBS path: allow -DOBS_PATH= on command line, else platform defaults
if(NOT DEFINED OBS_PATH)
    if(APPLE)
        set(OBS_PATH "$ENV{HOME}/Development/obs-studio")
    else()
        set(OBS_PATH "/opt/obs-studio")
    endif()
endif()
set(OBS_SOURCE_PATH "${OBS_PATH}")

# OBS include directories from source
set(OBS_INCLUDE_DIRS "${OBS_SOURCE_PATH}/libobs")

if(APPLE)
    # macOS: link against OBS.app (installed or from build dir for CI)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks;@executable_path/../../../../Frameworks")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(OBS_APP_PATH "/Applications/OBS.app" CACHE PATH "Path to OBS.app (or build output)")
    if(DEFINED OBS_LIBRARY AND OBS_LIBRARY)
        # CI or custom: use explicit path
    elseif(DEFINED OBS_BUILD_DIR AND OBS_BUILD_DIR)
        # Build dir: try common OBS macOS output locations
        set(_obs_fw "${OBS_BUILD_DIR}/frontend/Release/OBS.app/Contents/Frameworks/libobs.framework/libobs")
        if(EXISTS "${_obs_fw}")
            set(OBS_LIBRARY "${_obs_fw}")
        elseif(EXISTS "${OBS_BUILD_DIR}/rundir/Release/OBS.app/Contents/Frameworks/libobs.framework/libobs")
            set(OBS_LIBRARY "${OBS_BUILD_DIR}/rundir/Release/OBS.app/Contents/Frameworks/libobs.framework/libobs")
        else()
            set(OBS_LIBRARY "${OBS_APP_PATH}/Contents/Frameworks/libobs.framework/libobs")
        endif()
    else()
        set(OBS_LIBRARY "${OBS_APP_PATH}/Contents/Frameworks/libobs.framework/libobs")
    endif()
else()
    # Linux: link against OBS build directory (e.g. /opt/obs-studio/build/libobs)
    if(NOT DEFINED OBS_BUILD_DIR)
        set(OBS_BUILD_DIR "${OBS_SOURCE_PATH}/build")
    endif()
    set(OBS_LIBRARY "${OBS_BUILD_DIR}/libobs/libobs.so")
endif()

# Verify headers exist
if(NOT EXISTS "${OBS_SOURCE_PATH}/libobs/obs-module.h")
    message(FATAL_ERROR "OBS source not found at ${OBS_SOURCE_PATH}. Set -DOBS_PATH=/path/to/obs-studio")
endif()

# Verify library exists (Linux may have libobs.so or libobs.so.0)
if(APPLE)
    if(NOT EXISTS "${OBS_LIBRARY}")
        message(FATAL_ERROR "libobs not found at ${OBS_LIBRARY}. Install OBS Studio, or set -DOBS_PATH= and -DOBS_BUILD_DIR= or -DOBS_LIBRARY=.")
    endif()
else()
    if(NOT EXISTS "${OBS_LIBRARY}")
        if(EXISTS "${OBS_BUILD_DIR}/libobs/libobs.so.0")
            set(OBS_LIBRARY "${OBS_BUILD_DIR}/libobs/libobs.so.0")
        else()
            message(FATAL_ERROR "libobs not found. Build OBS first in ${OBS_BUILD_DIR} or set -DOBS_BUILD_DIR=.")
        endif()
    endif()
endif()

# Find nlohmann_json
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Optional: dock panel (View → Docks) requires Qt and OBS frontend API
set(LOWERTHIRDS_HAS_DOCK OFF)
if(APPLE)
    if(DEFINED OBS_BUILD_DIR AND OBS_BUILD_DIR)
        set(_fe "${OBS_BUILD_DIR}/frontend/Release/OBS.app/Contents/Frameworks/obs-frontend-api.dylib")
        if(EXISTS "${_fe}")
            set(OBS_FRONTEND_LIBRARY "${_fe}")
        else()
            set(OBS_FRONTEND_LIBRARY "${OBS_APP_PATH}/Contents/Frameworks/obs-frontend-api.dylib")
        endif()
    else()
        set(OBS_FRONTEND_LIBRARY "${OBS_APP_PATH}/Contents/Frameworks/obs-frontend-api.dylib")
    endif()
else()
    set(OBS_FRONTEND_LIBRARY "${OBS_BUILD_DIR}/obs-frontend-api/libobs-frontend-api.so" CACHE PATH "OBS frontend API for dock")
    if(NOT EXISTS "${OBS_FRONTEND_LIBRARY}")
        set(OBS_FRONTEND_LIBRARY "${OBS_BUILD_DIR}/UI/obs-frontend-api/libobs-frontend-api.so")
    endif()
endif()
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets QUIET)
endif()
# Dock is off by default (on macOS, Qt from Homebrew can conflict with OBS.app's Qt)
set(LOWERTHIRDS_BUILD_DOCK OFF CACHE BOOL "Build dock panel (View → Docks). Requires matching Qt.")
if(LOWERTHIRDS_BUILD_DOCK AND (Qt6_FOUND OR Qt5_FOUND) AND EXISTS "${OBS_FRONTEND_LIBRARY}")
    set(LOWERTHIRDS_HAS_DOCK ON)
    message(STATUS "LowerThirdsPlus: building with dock panel (View → Docks → Lower Thirds Control)")
else()
    if(NOT LOWERTHIRDS_HAS_DOCK)
        message(STATUS "LowerThirdsPlus: no dock (use -DLOWERTHIRDS_BUILD_DOCK=ON to enable)")
    endif()
endif()

# Source files
set(PLUGIN_SOURCES
    src/plugin-main-simple.cpp
    src/lowerthirds-source-simple.cpp
    src/json-loader.cpp
)
set(PLUGIN_HEADERS
    src/lowerthirds-source-simple.hpp
    src/json-loader.hpp
)
if(LOWERTHIRDS_HAS_DOCK)
    list(APPEND PLUGIN_SOURCES src/lowerthirds-dock.cpp)
    list(APPEND PLUGIN_HEADERS src/lowerthirds-dock.hpp)
endif()

# Create plugin library
add_library(LowerThirdsPlus MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)
set_target_properties(LowerThirdsPlus PROPERTIES AUTOMOC ON)

# Include directories
target_include_directories(LowerThirdsPlus PRIVATE
    ${OBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/obs-headers
)

# Link libraries
target_link_libraries(LowerThirdsPlus PRIVATE
    nlohmann_json::nlohmann_json
    ${OBS_LIBRARY}
)
if(LOWERTHIRDS_HAS_DOCK)
    target_compile_definitions(LowerThirdsPlus PRIVATE LOWERTHIRDS_HAS_DOCK)
    if(Qt6_FOUND)
        # Use Qt include dirs first so compiler does not pick OBS.app's Qt from -F path
        target_include_directories(LowerThirdsPlus PRIVATE BEFORE
            $<TARGET_PROPERTY:Qt6::Widgets,INTERFACE_INCLUDE_DIRECTORIES>)
        target_link_libraries(LowerThirdsPlus PRIVATE Qt6::Widgets)
    else()
        target_include_directories(LowerThirdsPlus PRIVATE BEFORE
            $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>)
        target_link_libraries(LowerThirdsPlus PRIVATE Qt5::Widgets)
    endif()
    target_link_libraries(LowerThirdsPlus PRIVATE "${OBS_FRONTEND_LIBRARY}")
endif()

if(APPLE)
    # macOS: bundle plugin
    set_target_properties(LowerThirdsPlus PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "plugin"
        OUTPUT_NAME "LowerThirdsPlus"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
    install(TARGETS LowerThirdsPlus
        LIBRARY DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins"
        BUNDLE DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins"
    )
else()
    # Linux: shared module
    set_target_properties(LowerThirdsPlus PROPERTIES
        PREFIX ""
        OUTPUT_NAME "LowerThirdsPlus"
    )
    # OBS on Linux looks for plugins in lib/obs-plugins (relative to OBS prefix) or build dir
    install(TARGETS LowerThirdsPlus
        LIBRARY DESTINATION "lib/obs-plugins"
    )
endif()

# Compiler warnings
target_compile_options(LowerThirdsPlus PRIVATE -Wall -Wextra -Wpedantic)

# Print configuration info
message(STATUS "=== LowerThirdsPlus Configuration ===")
message(STATUS "OBS Source: ${OBS_SOURCE_PATH}")
message(STATUS "OBS Library: ${OBS_LIBRARY}")
message(STATUS "=====================================")
