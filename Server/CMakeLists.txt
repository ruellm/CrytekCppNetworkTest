cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DUNIX)

add_executable(Server
        Server.cpp
        EchoDistributedServer.cpp
        Thread/ThreadPool.cpp
        Config/PeersConfigLoader.cpp
        ../Common/ConfigLoader.cpp
        ../Common/SocketFactory.cpp
        ../Common/PosixSocket.cpp
        ../Common/ByteHandler.cpp
        ../Common/Packet.cpp
        ../Common/PacketBuilder.cpp
        ../Common/PacketSender.cpp
        ../Common/PacketStringMessage.cpp
        ../Common/Tokenizer.cpp
	../Common/PacketReceiver.cpp
        )

target_include_directories(Server PUBLIC ../ )

target_link_libraries(Server PUBLIC pthread)
