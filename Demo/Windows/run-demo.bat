:: Run 3 servers
start Server.exe --id server_1 --port 9000 --peers ../peers.txt --type 1
start Server.exe --id server_2 --port 9002 --peers ../peers.txt --type 2
start Server.exe --id server_3 --port 9004 --peers ../peers.txt --type 0

::Add thread delay to make sure peers are connectecd to each other before we start the clients
TIMEOUT 5

:: Run Listener clients
start Client.exe --id client_5 --host localhost --port 9000 --listener
start Client.exe --id client_6 --host localhost --port 9002 --listener
start Client.exe --id client_7 --host localhost --port 9004 --listener

:: Run clients connected to each servers
start Client.exe --id client_1 --host localhost --port 9000 --frequency -1 --delay 0 --message "Hello from client 1"
start Client.exe --id client_2 --host localhost --port 9002 --frequency -1 --delay 0 --message "Hello from client 2"
start Client.exe --id client_3 --host localhost --port 9004 --frequency 10 --delay 3 --message "Hello from client 3"
start Client.exe --id client_4 --host localhost --port 9002 --frequency -1 --delay 1 --message "Client 4 says hi"


