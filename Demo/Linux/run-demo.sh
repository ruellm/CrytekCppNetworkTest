#!/bin/bash

# Run 3 servers
gnome-terminal -- bash -c "./Server --id server_1 --port 9000; exec bash"
gnome-terminal -- bash -c "./Server --id server_2 --port 9002 --peers ../peers.txt; exec bash"
gnome-terminal -- bash -c "./Server --id server_3 --port 9004 --peers ../peers.txt; exec bash"

# Run 3 servers
gnome-terminal -- bash -c "./Client --id client_1 --host localhost --port 9000 --frequency -1 --delay 0 --message \"Hello from client 1\"; exec bash"
gnome-terminal -- bash -c "./Client --id client_2 --host localhost --port 9002 --frequency -1 --delay 2 --message \"Hello from client 2\"; exec bash"
gnome-terminal -- bash -c "./Client --id client_3 --host localhost --port 9004 --frequency 10 --delay 3 --message \"Hello from client 3\"; exec bash"
gnome-terminal -- bash -c "./Client --id client_4 --host localhost --port 9002 --frequency -1 --delay 3 --message \"Client 4 says hi\"; exec bash"

# Run Listener clients
gnome-terminal -- bash -c  "./Client --id client_5 --host localhost --port 9002 --listener; exec bash"
gnome-terminal -- bash -c  "./Client --id client_6 --host localhost --port 9004 --listener; exec bash"
