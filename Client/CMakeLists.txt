cmake_minimum_required(VERSION 3.10)
project(Client)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DUNIX)

add_executable(Client
        Client.cpp
        Config/ServerListLoader.cpp
        ../Common/ConfigLoader.cpp
        ../Common/SocketFactory.cpp
        ../Common/PosixSocket.cpp
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
        Test/Test.cpp
        )

target_include_directories(Client PUBLIC ../ )
target_link_libraries(Client PUBLIC pthread)
