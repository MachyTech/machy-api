cmake_minimum_required(VERSION 3.20.1)

# name of the project
project(machyapi)

#enable language
enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)
# pthread
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(include)

# sources
file(GLOB SOURCES "src/*.cpp")

# test csv file
file (COPY samples/trajectory_1_fpg_out.txt DESTINATION .)


#link libraries
find_package( Boost 1.58 EXACT REQUIRED system )
include_directories(${Boost_INCLUDE_DIRS})

if(BUILD_EXAMPLES)
    add_executable(async_trajectoryclient "samples/async_trajectoryclient.cpp" ${SOURCES})
    target_link_libraries(async_trajectoryclient ${Boost_LIBRARIES})
    add_executable(async_positionclient "samples/async_posclient.cpp" ${SOURCES})
    target_link_libraries(async_positionclient ${Boost_LIBRARIES})
endif()
