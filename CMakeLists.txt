cmake_minimum_required(VERSION 3.10)

project(megamarienhancer)

find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)

add_library(hook32 SHARED hook.cpp)
add_executable(launcher launcher.cpp)

set_target_properties(hook32 PROPERTIES LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/hook.def")

target_include_directories(hook32 PRIVATE ${DETOURS_INCLUDE_DIRS})
target_link_libraries(hook32 PRIVATE ${DETOURS_LIBRARY})

target_include_directories(launcher PRIVATE ${DETOURS_INCLUDE_DIRS})
target_link_libraries(launcher PRIVATE ${DETOURS_LIBRARY})