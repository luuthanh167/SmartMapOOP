cmake_minimum_required(VERSION 3.16)
project(SmartMapOOP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Models
    ${CMAKE_CURRENT_SOURCE_DIR}/Graph
    ${CMAKE_CURRENT_SOURCE_DIR}/Service
    ${CMAKE_CURRENT_SOURCE_DIR}/Server
)

set(SOURCES
    Models/Position.cpp
    Models/Location.cpp
    Models/Preference.cpp
    Models/User.cpp
    Graph/Edge.cpp
    Graph/Graph.cpp
    Graph/PathFinder.cpp
    Graph/RouteOptimizer.cpp
    Graph/RouteResult.cpp
    Service/DistanceService.cpp
    Service/RoutingService.cpp
    Service/PlaceService.cpp
    Server/HttpServer.cpp
    main.cpp
)

add_executable(SmartMapOOP ${SOURCES})

if(WIN32)
    target_link_libraries(SmartMapOOP ws2_32 shell32)
endif()
