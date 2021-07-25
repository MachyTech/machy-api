cmake_minimum_required(VERSION 3.20.1)

# name of the project
project(machyapi VERSION 0.0.1 DESCRIPTION "machyapi functions used by machytechs projecting system")

add_library(machyapi SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/machyapi.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/machysockets.cpp)

set_target_properties(machyapi PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machyapi PROPERTIES SOVERSION 0.0.1)

target_include_directories(machyapi PUBLIC ${CMAKE_SOURCE_DIR}/include)

#link libraries
find_package( Boost 1.58 EXACT REQUIRED system )
include_directories(${Boost_INCLUDE_DIRS})

# build with examples
if(BUILD_EXAMPLES)
    # test csv file
    file (COPY samples/trajectory_1_fpg_out.txt DESTINATION .)
    add_executable(async_trajectoryclient "samples/async_trajectoryclient.cpp" ${SOURCES})
    target_link_libraries(async_trajectoryclient ${Boost_LIBRARIES})
    add_executable(async_positionclient "samples/async_posclient.cpp" ${SOURCES})
    target_link_libraries(async_positionclient ${Boost_LIBRARIES})
endif()

# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machyapi
        EXPORT machyapiTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
