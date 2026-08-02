cmake_minimum_required(VERSION 3.14...4.0)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(userver-modbus CXX)

find_package(userver COMPONENTS core REQUIRED)

userver_setup_environment()

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp)
file(GLOB_RECURSE UNIT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*_test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(${PROJECT_NAME}_objs OBJECT ${SOURCES})
include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core)

add_executable(${PROJECT_NAME}_unittest ${UNIT_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)

add_library(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX ${PREFIX_PATH})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${PROJECT_NAME})
