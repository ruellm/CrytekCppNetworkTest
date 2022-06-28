:: Run 3 servers
start Server.exe --id server_1 --port 9000
start Server.exe --id server_2 --port 9002 --peers peers.txt
start Server.exe --id server_3 --port 9004 --peers peers.txt

:: Run clients connected to each servers
start Client.exe --id client_1 --host localhost --port 9000 --frequency -1 --delay 1 --message "Hello from client 1"
start Client.exe --id client_2 --host localhost --port 9002 --frequency -1 --delay 2 --message "Hello from client 2"
start Client.exe --id client_3 --host localhost --port 9004 --frequency 10 --delay 3 --message "Hello from client 3"
start Client.exe --id client_4 --host localhost --port 9002 --frequency -1 --delay 1 --message "Client 4 says hi"

