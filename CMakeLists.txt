cmake_minimum_required(VERSION 3.12)

project(easyspdlog)

set(SPDLOG_ACTIVE_LEVEL "SPDLOG_LEVEL_TRACE" CACHE STRING "SPDLOG_ACTIVE_LEVEL for compile time logging")
set_property(CACHE SPDLOG_ACTIVE_LEVEL PROPERTY STRINGS SPDLOG_LEVEL_OFF SPDLOG_LEVEL_TRACE SPDLOG_LEVEL_DEBUG SPDLOG_LEVEL_INFO SPDLOG_LEVEL_WARN SPDLOG_LEVEL_ERROR SPDLOG_LEVEL_CRITICAL)
set(SPDLOG_ACTIVE_LEVEL "SPDLOG_LEVEL_TRACE" CACHE STRING "SPDLOG_ACTIVE_LEVEL for compile time logging")

add_definitions(-DSPDLOG_ACTIVE_LEVEL=${SPDLOG_ACTIVE_LEVEL})

set(CPM_USE_LOCAL_PACKAGES ON)
# set(CPM_LOCAL_PACKAGES_ONLY ON) add dependencies
include(cmake/CPM.cmake)

option(BUILD_WITH_SPDLOG "Build with spdlog?" FALSE)

if (BUILD_WITH_SPDLOG)
    cpmaddpackage("gh:gabime/spdlog@1.2.0")
    add_compile_definitions(MSGX_BUILD_WITH_SPDLOG=1)
endif ()

cpmaddpackage(
        GITHUB_REPOSITORY soraxas/cpp-soraxas_toolbox
        GIT_TAG master
        GIT_SHALLOW True
        GIT_SUBMODULES "include/soraxas_toolbox/external/_repo/concurrentqueue"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_PROGRESS TRUE
        OPTIONS
        "BUILD_SXS_WITH_TESTS Off"
)


add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE soraxas_toolbox spdlog::spdlog)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)


INSTALL(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")

