cmake_minimum_required(VERSION 3.10)
project(Client)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DUNIX)

add_executable(Client
        Client.cpp
        ServerListLoader.cpp
        ../Common/ConfigLoader.cpp
        ../Common/SocketFactory.cpp
        ../Common/PosixSocket.cpp
        Test/Test.cpp
        Network/Packet.cpp
        Network/PacketBuilder.cpp
        )

target_include_directories(Client PUBLIC ../ )
target_link_libraries(Client PUBLIC pthread)