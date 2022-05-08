cmake_minimum_required(VERSION 3.0.0)
project(eyfs VERSION 0.1.0)

aux_source_directory(./src DIR_ROOT)
include_directories(./include)
add_executable (${PROJECT_NAME} ${DIR_ROOT})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
