cmake_minimum_required(VERSION 3.15)

project(user LANGUAGES CXX)

add_executable(app main.cpp)

if(ADD_SUBDIRECTORY)
    add_subdirectory("${ADD_SUBDIRECTORY}")
else()
    find_package(cntgs 0.1 REQUIRED)
endif()

target_link_libraries(app PRIVATE cntgs::cntgs)

install(TARGETS app RUNTIME DESTINATION bin)
