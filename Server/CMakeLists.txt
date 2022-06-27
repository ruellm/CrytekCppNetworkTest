cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DUNIX)

add_executable(Server
        Server.cpp
        EchoDistributedServer.cpp
        ThreadPool.cpp
        Config/PeersConfigLoader.cpp
        Config/ServerConfigLoader.cpp
        ../Common/ConfigLoader.cpp
        ../Common/SocketFactory.cpp
        ../Common/PosixSocket.cpp
        )

target_include_directories(Server PUBLIC ../ )

target_link_libraries(Server PUBLIC pthread)
